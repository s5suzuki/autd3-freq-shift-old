/*
 * File: pwm_generator.sv
 * Project: transducer
 * Created Date: 15/12/2020
 * Author: Shun Suzuki
 * -----
 * Last Modified: 12/04/2021
 * Modified By: Shun Suzuki (suzuki@hapis.k.u-tokyo.ac.jp)
 * -----
 * Copyright (c) 2020 Hapis Lab. All rights reserved.
 * 
 */

`timescale 1ns / 1ps

module pwm_generator(
           input var [8:0] TIME,
           input var [7:0] DUTY,
           input var [7:0] PHASE,
           output var PWM_OUT
       );

logic [9:0] t;
logic [9:0] duty;
logic [9:0] s;

assign t = {1'b0, TIME};
assign duty = {2'b00, DUTY};
assign s = keep_phase(PHASE, duty); // s is a shift duration of PWM signal
assign PWM_OUT = pwm(t, duty, s);

function automatic pwm;
    input [9:0] t;
    input [9:0] duty;
    input [9:0] s;
    begin
        if (duty + s < 10'd510) begin
            pwm = (s <= t) & (t < duty + s);
        end
        else begin
            pwm = (s <= t) | ((s <= t + 10'd510) & (t + 10'd510 < duty + s));
        end
    end
endfunction

// Shift duration S does not equal the phase of ultrasound emitted and has some a bias term related to duty ratio D.
// (c.f. Eq. 16 in "Suzuki, Shun, et al. "Reducing Amplitude Fluctuation by Gradual Phase Shift in Midair Ultrasound Haptics." IEEE Transactions on Haptics 13.1 (2020): 87-93.")
// Therefore, even if the phase is constant, S will change as D changes.
function automatic [9:0] keep_phase;
    input [7:0] phase;
    input [7:0] duty;
    keep_phase = {1'b0, phase, 1'b0} + (10'h07F - {2'b00, duty[7:1]});
endfunction

endmodule
