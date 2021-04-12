/*
 * File: top.sv
 * Project: new
 * Created Date: 27/03/2021
 * Author: Shun Suzuki
 * -----
 * Last Modified: 12/04/2021
 * Modified By: Shun Suzuki (suzuki@hapis.k.u-tokyo.ac.jp)
 * -----
 * Copyright (c) 2021 Hapis Lab. All rights reserved.
 * 
 */

`timescale 1ns / 1ps

module top(
           input var [16:0] CPU_ADDR,
           inout tri [15:0] CPU_DATA,
           input var CPU_CKIO,
           input var CPU_CS1_N,
           input var RESET_N,
           input var CPU_WE0_N,
           input var CPU_WE1_N,
           input var CPU_RD_N,
           input var CPU_RDWR,
           input var MRCC_25P6M,
           input var CAT_SYNC0,
           output var FORCE_FAN,
           input var THERMO,
           input var [3:0]GPIO_IN,
           output var [252:1] XDCR_OUT,
           output var [3:0]GPIO_OUT
       );

localparam int TRANS_NUM = 249;

localparam int RESOLUTION_WIDTH = 8;
localparam [RESOLUTION_WIDTH-1:0] DUTY_MAX = 255;
localparam [RESOLUTION_WIDTH+1:0] TIME_CNT_CYCLE = 2*DUTY_MAX;

localparam int SYS_CLK_FREQ = 20400000;
localparam int ULTRASOUND_FREQ = SYS_CLK_FREQ/TIME_CNT_CYCLE;

localparam int MOD_FREQ = 4000;
localparam int MOD_BUF_SIZE = 4000;
localparam int MOD_CNT_CYCLE = SYS_CLK_FREQ/MOD_FREQ;

localparam [3:0] BRAM_TR_SELECT = 4'h0;
localparam [3:0] BRAM_MOD_SELECT = 4'h1;
localparam [3:0] BRAM_CONFIG_SELECT = 4'hF;

localparam int CTRL_FLAGS_ADDR = 0;
localparam int FPGA_INFO_ADDR = 1;
localparam int CLK_SYNC_ADDR = 2;

localparam int SILENT_MODE_IDX = 3;
localparam int FORCE_FAN_IDX = 4;

logic sys_clk;
logic bus_clk;
logic reset;

logic [RESOLUTION_WIDTH+1:0] time_cnt_for_ultrasound;

logic [3:0] cpu_select;
logic [11:0] cpu_addr;
logic [15:0] cpu_data_out;
logic tr_wea;
logic mod_wea;
logic config_wea;

logic [RESOLUTION_WIDTH-1:0] duty[0:TRANS_NUM-1];
logic [RESOLUTION_WIDTH-1:0] phase[0:TRANS_NUM-1];
logic [RESOLUTION_WIDTH-1:0] mod;

logic [2:0] sync0;

logic [7:0] ctrl_flags;
logic [7:0] fpga_info;
logic silent;

assign bus_clk = CPU_CKIO;
assign reset = ~RESET_N;
assign cpu_select = CPU_ADDR[16:13];
assign cpu_addr = CPU_ADDR[12:1];
assign CPU_DATA  = (~CPU_CS1_N && ~CPU_RD_N && CPU_RDWR) ? cpu_data_out : 16'bz;
assign tr_wea = (cpu_select == BRAM_TR_SELECT) & (~CPU_WE0_N);
assign mod_wea = (cpu_select == BRAM_MOD_SELECT) & (~CPU_WE0_N);

assign silent = ctrl_flags[SILENT_MODE_IDX];
assign FORCE_FAN = ctrl_flags[FORCE_FAN_IDX];
assign fpga_info = {7'd0, THERMO};

ultrasound_cnt_clk_gen ultrasound_cnt_clk_gen(
                           .clk_in1(MRCC_25P6M),
                           .reset(reset),
                           .clk_out1(sys_clk),
                           .clk_out2(lpf_clk)
                       );

//////////////////////////////// Duty and Phase set ////////////////////////////////////////
logic [7:0] tr_cnt_write;
logic [7:0] tr_bram_addr;
logic [15:0] tr_bram_dataout;
logic [RESOLUTION_WIDTH:0] duty_buf[0:TRANS_NUM-1];
logic [RESOLUTION_WIDTH:0] phase_buf[0:TRANS_NUM-1];

enum logic [2:0] {
         IDLE,
         DUTY_PHASE_WAIT_0,
         DUTY_PHASE_WAIT_1,
         DUTY_PHASE
     } tr_state;

tr_bram tr_bram(
            .clka(bus_clk),
            .ena(~CPU_CS1_N),
            .wea(tr_wea),
            .addra(cpu_addr[7:0]),
            .dina(CPU_DATA),
            .douta(),
            .clkb(sys_clk),
            .web(1'b0),
            .addrb(tr_bram_addr),
            .dinb(16'h0000),
            .doutb(tr_bram_dataout)
        );

always_ff @(posedge sys_clk) begin
    if (reset) begin
        tr_bram_addr <= 0;
        tr_state <= IDLE;
        tr_cnt_write <= 0;
        duty_buf <= '{TRANS_NUM{8'h00}};
        phase_buf <= '{TRANS_NUM{8'h00}};
    end
    else begin
        case(tr_state)
            IDLE: begin
                if (TIME == 10'd0) begin
                    tr_bram_addr <= 8'd0;
                    tr_state <= DUTY_PHASE_WAIT_0;
                end
            end
            DUTY_PHASE_WAIT_0: begin
                tr_bram_addr <= tr_bram_addr + 1;
                tr_state <= DUTY_PHASE_WAIT_1;
            end
            DUTY_PHASE_WAIT_1: begin
                tr_bram_addr <= tr_bram_addr + 1;
                tr_cnt_write <= 0;
                tr_state <= DUTY_PHASE;
            end
            DUTY_PHASE: begin
                duty_buf[tr_cnt_write] <= tr_bram_dataout[15:8];
                phase_buf[tr_cnt_write] <= tr_bram_dataout[7:0];
                if (tr_cnt_write == TRANS_NUM - 1) begin
                    tr_bram_addr <= 8'd0;
                    tr_state <= IDLE;
                end
                else begin
                    tr_bram_addr <= tr_bram_addr + 1;
                    tr_cnt_write <= tr_cnt_write + 1;
                    tr_state <= DUTY_PHASE;
                end
            end
        endcase
    end
end

always_ff @(posedge sys_clk) begin
    if (reset) begin
        duty <= '{TRANS_NUM{8'h00}};
        phase <= '{TRANS_NUM{8'h00}};
    end
    else if (time_cnt_for_ultrasound == (TIME_CNT_MAX - 1)) begin
        duty <= duty_buf;
        phase <= phase_buf;
    end
end
//////////////////////////////// Duty and Phase set ////////////////////////////////////////

//////////////////////////////// Modulation control ////////////////////////////////////////
logic [11:0] mod_bram_addr;
logic [12:0] mod_cnt;
mod_bram mod_bram(
             .clka(bus_clk),
             .ena(~CPU_CS1_N),
             .wea(mod_wea),
             .addra(cpu_addr[10:0]),
             .dina(CPU_DATA),
             .douta(),
             .clkb(sys_clk),
             .web(1'b0),
             .addrb(mod_bram_addr),
             .dinb(8'h00),
             .doutb(mod)
         );

always_ff @(posedge sys_clk) begin
    if (reset) begin
        mod_cnt <= 0;
        mod_bram_addr <= 0;
    end
    else begin
        if (mod_cnt == (MOD_CNT_CYCLE - 1)) begin
            mod_cnt <= 0;
            mod_bram_addr <= (mod_bram_addr == (MOD_BUF_SIZE - 1)) ? 0 : mod_bram_addr + 1;
        end
        else begin
            mod_cnt <= mod_cnt + 1;
        end
    end
end
//////////////////////////////// Modulation control ////////////////////////////////////////

////////////////////////////////// Config control //////////////////////////////////////////
logic [15:0] config_bram_din;

enum logic [2:0] {
         CTRL_FLAGS_READ,
         FPGA_INFO_WRITE,
         CLK_SYNC_READ
     } config_state;

config_bram config_bram(
                .clka(bus_clk),
                .ena(~CPU_CS1_N),
                .wea(config_wea),
                .addra(cpu_addr[7:0]),
                .dina(CPU_DATA),
                .douta(cpu_data_out),
                .clkb(sys_clk),
                .web(config_web),
                .addrb(config_bram_addr),
                .dinb(config_bram_din),
                .doutb(config_bram_dout)
            );


always_ff @(posedge sys_clk) begin
    if (reset) begin
        config_state <= CTRL_FLAGS_READ;
    end
    else begin
        case(config_state)
            CTRL_FLAGS_READ: begin
                config_bram_addr <= CTRL_FLAGS_ADDR;
                ctrl_flags <= config_bram_dout[7:0];
                config_state <= FPGA_INFO_WRITE;
            end
            FPGA_INFO_WRITE: begin
                config_bram_addr <= FPGA_INFO_ADDR;
                config_bram_din <= fpga_info;
                config_web <= 1'b1;
                config_state <= CLK_SYNC_READ;
            end
            CLK_SYNC_READ: begin
                config_bram_addr <= CLK_SYNC_ADDR;
                config_web <= 1'b0;
                config_state <= CTRL_FLAGS_READ;
            end
        endcase
    end
end
////////////////////////////////// Config control //////////////////////////////////////////

`include "cvt_uid.vh"
generate begin:TRANSDUCERS_GEN
        genvar ii;
        for(ii = 0; ii < TRANS_NUM; ii++) begin
            logic [7:0] duty_modulated;
            assign duty_modulated = modulate_duty(duty[ii], mod);
            transducer tr(
                           .CLK(sys_clk),
                           .RST(reset),
                           .CLK_LPF(lpf_clk),
                           .TIME(time_cnt_for_ultrasound),
                           .DUTY(duty_modulated),
                           .PHASE(phase[ii]),
                           .SILENT(silent),
                           .PWM_OUT(XDCR_OUT[cvt_uid(ii) + 1])
                       );
        end
    end
endgenerate

function automatic [7:0] modulate_duty;
    input [7:0] duty;
    input [7:0] mod;
    modulate_duty = ((duty + 17'd1) * (mod + 17'd1) - 17'd1) >> 8;
endfunction

endmodule
