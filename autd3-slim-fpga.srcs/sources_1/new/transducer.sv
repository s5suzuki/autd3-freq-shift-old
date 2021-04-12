/*
 * File: transducer.sv
 * Project: new
 * Created Date: 03/10/2019
 * Author: Shun Suzuki
 * -----
 * Last Modified: 11/04/2021
 * Modified By: Shun Suzuki (suzuki@hapis.k.u-tokyo.ac.jp)
 * -----
 * Copyright (c) 2019 Hapis Lab. All rights reserved.
 * 
 */

`timescale 1ns / 1ps
module transducer#(
           parameter int RESOLUTION_WIDTH = 8,
           parameter [RESOLUTION_WIDTH-1:0] DUTY_MAX = 255,
           localparam [RESOLUTION_WIDTH+1:0] TIME_CNT_MAX = 2*DUTY_MAX
       )(
           input var CLK,
           input var RST,
           input var CLK_LPF,
           input var [RESOLUTION_WIDTH:0] TIME,
           input var [RESOLUTION_WIDTH-1:0] DUTY,
           input var [RESOLUTION_WIDTH-1:0] PHASE,
           input var SILENT,
           output var PWM_OUT
       );

logic[RESOLUTION_WIDTH-1:0] duty_s, phase_s;
logic[RESOLUTION_WIDTH-1:0] duty, phase;

assign duty = SILENT ? duty_s : DUTY;
assign phase = SILENT ? phase_s : PHASE;

assign update = (TIME == (TIME_CNT_MAX-1));

silent_lpf silent_lpf(
               .CLK(CLK),
               .RST(RST),
               .CLK_LPF(CLK_LPF),
               .UPDATE(update),
               .DUTY(DUTY),
               .PHASE(PHASE),
               .DUTY_S(duty_s),
               .PHASE_S(phase_s)
           );

pwm_generator#(
                 .RESOLUTION_WIDTH(RESOLUTION_WIDTH),
                 .DUTY_MAX(DUTY_MAX),
                 .TIME_CNT_MAX(TIME_CNT_MAX)
             ) pwm_generator(
                 .TIME(TIME),
                 .DUTY(duty),
                 .PHASE(phase),
                 .PWM_OUT(PWM_OUT)
             );

endmodule
