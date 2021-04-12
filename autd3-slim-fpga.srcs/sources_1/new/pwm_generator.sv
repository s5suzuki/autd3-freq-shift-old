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

module pwm_generator#(
           parameter int RESOLUTION_WIDTH = 8,
           parameter [RESOLUTION_WIDTH-1:0] DUTY_MAX = 255,
           parameter [RESOLUTION_WIDTH-1:0] TIME_CNT_MAX = 2*DUTY_MAX
       )(
           input var [RESOLUTION_WIDTH:0] TIME,
           input var [RESOLUTION_WIDTH-1:0] DUTY,
           input var [RESOLUTION_WIDTH-1:0] PHASE,
           output var PWM_OUT
       );

logic [RESOLUTION_WIDTH+1:0] t;
logic [RESOLUTION_WIDTH+1:0] d;
logic [RESOLUTION_WIDTH+1:0] s;

assign t = {1'b0, TIME};
assign d = {2'b00, DUTY};
assign s = keep_phase(PHASE, d); // s is a shift duration of PWM signal
assign PWM_OUT = pwm(t, d, s);

function automatic pwm;
    input [RESOLUTION_WIDTH+1:0] t;
    input [RESOLUTION_WIDTH+1:0] d;
    input [RESOLUTION_WIDTH+1:0] s;
    begin
        if (d + s < TIME_CNT_MAX) begin
            pwm = (s <= t) & (t < d + s);
        end
        else begin
            pwm = (s <= t) | ((s <= t + TIME_CNT_MAX) & (t + TIME_CNT_MAX < d + s));
        end
    end
endfunction

// Shift duration S does not equal the phase of ultrasound emitted and has some a bias term related to duty ratio D.
// (c.f. Eq. 16 in "Suzuki, Shun, et al. "Reducing Amplitude Fluctuation by Gradual Phase Shift in Midair Ultrasound Haptics." IEEE Transactions on Haptics 13.1 (2020): 87-93.")
// Therefore, even if the phase is constant, S will change as D changes.
function automatic [RESOLUTION_WIDTH+1:0] keep_phase;
    input [RESOLUTION_WIDTH-1:0] phase;
    input [RESOLUTION_WIDTH-1:0] d;
    keep_phase = {1'b0, phase, 1'b0} + ({2'b0, DUTY_MAX[RESOLUTION_WIDTH-1:1]} - {2'b00, d[RESOLUTION_WIDTH-1:1]});
endfunction

endmodule
