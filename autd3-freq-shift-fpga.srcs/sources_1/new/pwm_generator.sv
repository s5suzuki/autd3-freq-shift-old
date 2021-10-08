/*
 * File: pwm_generator.sv
 * Project: transducer
 * Created Date: 15/12/2020
 * Author: Shun Suzuki
 * -----
 * Last Modified: 09/10/2021
 * Modified By: Shun Suzuki (suzuki@hapis.k.u-tokyo.ac.jp)
 * -----
 * Copyright (c) 2020 Hapis Lab. All rights reserved.
 * 
 */

`timescale 1ns / 1ps

module pwm_generator#(
           parameter int WIDTH = 16
       )(
           input var [WIDTH-1:0] TIME,
           input var [WIDTH-1:0] CYCLE,
           input var [WIDTH-1:0] DUTY,
           input var [WIDTH-1:0] PHASE_DELAY,
           output var PWM_OUT
       );

assign PWM_OUT = pwm({1'b0, TIME}, {2'b00, CYCLE}, {1'b0, DUTY}, {1'b0, PHASE_DELAY});

function automatic pwm;
    input [WIDTH:0] t;
    input [WIDTH+1:0] C;
    input [WIDTH:0] D;
    input [WIDTH:0] P;
    logic [WIDTH:0] DL = {1'b0, D[WIDTH:1]};
    logic [WIDTH:0] DR = {1'b0, D[WIDTH:1]} + {16'h0000, D[0]};
    logic pwm1 = {1'b0, P} <= {1'b0, t} + {1'b0, DL};
    logic pwm2 = {1'b0, t} < {1'b0, P} + {1'b0, DR};
    logic pwm1o = C + {1'b0, P} <= {1'b0, t} + {1'b0, DL};
    logic pwm2o = C + {1'b0, t} < {1'b0, P} + {1'b0, DR};
    pwm = (pwm1 & pwm2) | ((pwm2 | pwm1o) & (P < DL)) | ((pwm2o | pwm1) & (C < {1'b0, P} + {1'b0, DR}));
endfunction

endmodule
