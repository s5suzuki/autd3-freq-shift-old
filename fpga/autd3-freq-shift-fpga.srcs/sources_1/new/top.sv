/*
 * File: top.sv
 * Project: new
 * Created Date: 27/03/2021
 * Author: Shun Suzuki
 * -----
 * Last Modified: 10/10/2021
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
           output var [252:1] XDCR_OUT,
           input var [3:0]GPIO_IN,
           output var [3:0]GPIO_OUT
       );

localparam int WIDTH = 16;

localparam int TRANS_NUM = 249;

localparam [3:0] BRAM_TR_SELECT = 4'h0;
localparam [3:0] BRAM_CONFIG_SELECT = 4'hF;

localparam [7:0] CTRL_FLAGS_ADDR = 8'h0;
localparam [7:0] FPGA_INFO_ADDR = 8'h1;
localparam [7:0] CYCLE_ADDR = 8'h10;

logic sys_clk;
logic bus_clk;
logic reset;

logic [WIDTH-1:0] time_cnt_for_ultrasound;

logic [3:0] cpu_select;
logic [10:0] cpu_addr;
logic [15:0] cpu_data_out;
logic tr_wea;
logic config_wea;

logic [WIDTH-1:0] cycle;
logic [WIDTH-1:0] duty[0:TRANS_NUM-1];
logic [WIDTH-1:0] phase[0:TRANS_NUM-1];

logic [7:0] ctrl_flags;
logic [7:0] fpga_info;

logic clk_sync;

assign bus_clk = CPU_CKIO;
assign reset = ~RESET_N;
assign cpu_select = CPU_ADDR[16:13];
assign cpu_addr = CPU_ADDR[11:1];
assign CPU_DATA  = (~CPU_CS1_N && ~CPU_RD_N && CPU_RDWR) ? cpu_data_out : 16'bz;
assign tr_wea = (cpu_select == BRAM_TR_SELECT) & (~CPU_WE0_N);
assign config_wea = (cpu_select == BRAM_CONFIG_SELECT) & (~CPU_WE0_N);

assign fpga_info = {7'd0, THERMO};

ultrasound_cnt_clk_gen ultrasound_cnt_clk_gen(
                           .clk_in1(MRCC_25P6M),
                           .reset(reset),
                           .clk_out1(sys_clk)
                       );

//////////////////////////////////// Synchronize ///////////////////////////////////////////
logic [2:0] sync0;
logic sync0_edge;

assign sync0_edge = (sync0 == 3'b011);

always_ff @(posedge sys_clk) begin
    sync0 <= reset ? 0 : {sync0[1:0], CAT_SYNC0};
    time_cnt_for_ultrasound <= (reset | sync0_edge | time_cnt_for_ultrasound == (cycle - 1)) ? 0 : time_cnt_for_ultrasound + 1;
end
//////////////////////////////////// Synchronize ///////////////////////////////////////////

//////////////////////////////// Duty and Phase set ////////////////////////////////////////
logic [7:0] tr_cnt_write;
logic [7:0] tr_bram_addr;
logic [31:0] tr_bram_dataout;
logic [WIDTH-1:0] duty_buf[0:TRANS_NUM-1];
logic [WIDTH-1:0] phase_buf[0:TRANS_NUM-1];

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
            .addra(cpu_addr[8:0]),
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
        duty_buf <= '{TRANS_NUM{0}};
        phase_buf <= '{TRANS_NUM{0}};
    end
    else begin
        case(tr_state)
            IDLE: begin
                if (time_cnt_for_ultrasound == 10'd0) begin
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
                duty_buf[tr_cnt_write] <= tr_bram_dataout[16+WIDTH-1:16];
                phase_buf[tr_cnt_write] <= tr_bram_dataout[WIDTH-1:0];
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
        duty <= '{TRANS_NUM{0}};
        phase <= '{TRANS_NUM{0}};
    end
    else if (time_cnt_for_ultrasound == (cycle - 1)) begin
        duty <= duty_buf;
        phase <= phase_buf;
    end
end
//////////////////////////////// Duty and Phase set ////////////////////////////////////////

////////////////////////////////// Config control //////////////////////////////////////////
logic [15:0] config_bram_din;
logic [15:0] config_bram_dout;
logic [7:0] config_bram_addr;
logic config_web;

enum logic [2:0] {
         CTRL_FLAGS_READ,
         FPGA_INFO_WRITE,
         CYCLE_READ
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
        config_bram_addr <= 0;
        config_bram_din <= 0;
        config_web <= 0;
        ctrl_flags <= 0;
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
                config_state <= CYCLE_READ;
            end
            CYCLE_READ: begin
                config_bram_addr <= CYCLE_ADDR;
                cycle <= config_bram_dout[WIDTH-1:0];
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
            logic [WIDTH-1:0] c;
            logic [WIDTH-1:0] t;
            pwm_generator#(
                             .WIDTH(WIDTH)
                         ) pwm_generator(
                             .CLK(sys_clk),
                             .TIME(t),
                             .CYCLE(c),
                             .DUTY(duty[ii]),
                             .PHASE_DELAY(phase[ii]),
                             .PWM_OUT(XDCR_OUT[cvt_uid(ii) + 1])
                         );
            always_ff @(posedge sys_clk) begin
                t <= time_cnt_for_ultrasound;
                c <= cycle;
            end
        end
    end
endgenerate

/////////////////////////////////////// Debug //////////////////////////////////////////////
logic gpo_0, gpo_1, gpo_2, gpo_3;

assign GPIO_OUT = {gpo_3, gpo_2, gpo_1, gpo_0};

always_ff @(posedge sys_clk) begin
    if (reset) begin
        gpo_0 <= 0;
        gpo_1 <= 0;
        gpo_2 <= 0;
        gpo_3 <= 0;
    end
    else begin
        gpo_0 <= (sync0_edge) ? ~gpo_0 : gpo_0;
        gpo_1 <= (time_cnt_for_ultrasound == 0) ? ~gpo_1 : gpo_1;
    end
end
/////////////////////////////////////// Debug //////////////////////////////////////////////

endmodule
