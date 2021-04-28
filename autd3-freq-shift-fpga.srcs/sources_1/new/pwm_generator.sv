/*
 * File: pwm_generator.sv
 * Project: transducer
 * Created Date: 15/12/2020
 * Author: Shun Suzuki
 * -----
 * Last Modified: 26/04/2021
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

assign PWM_OUT = pwm({1'b0, TIME}, {1'b0, CYCLE}, {1'b0, DUTY}, {1'b0, PHASE_DELAY});

function automatic pwm;
    input [WIDTH:0] t;
    input [WIDTH:0] c;
    input [WIDTH:0] d;
    input [WIDTH:0] s;
    begin
        if (s < (d>>1)) begin
            pwm = (s + c <= t + (d>>1)) | (t < s + (d+1>>1));
        end
        else if (s + (d+1>>1) > c) begin
            pwm = (s <= t + (d>>1)) | (t + c < s + (d+1>>1));
        end
        else begin
            pwm = (s <= t + (d>>1)) & (t < s + (d+1>>1));
        end
    end
endfunction

endmodule
