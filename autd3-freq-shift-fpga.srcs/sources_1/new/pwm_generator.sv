/*
 * File: pwm_generator.sv
 * Project: transducer
 * Created Date: 15/12/2020
 * Author: Shun Suzuki
 * -----
 * Last Modified: 10/10/2021
 * Modified By: Shun Suzuki (suzuki@hapis.k.u-tokyo.ac.jp)
 * -----
 * Copyright (c) 2020 Hapis Lab. All rights reserved.
 * 
 */

`timescale 1ns / 1ps

module pwm_generator#(
           parameter int WIDTH = 16
       )(
           input var CLK,
           input var [WIDTH-1:0] TIME,
           input var [WIDTH-1:0] CYCLE,
           input var [WIDTH-1:0] DUTY,
           input var [WIDTH-1:0] PHASE_DELAY,
           output var PWM_OUT
       );

logic [WIDTH:0] t;
logic [WIDTH+1:0] C;
logic [WIDTH:0] P;
logic [WIDTH:0] DL;
logic [WIDTH:0] DR;
logic [WIDTH+1:0] t2, p2, dl2, c2;
logic [WIDTH+1:0] pc, tc;
logic [WIDTH+1:0] left, right;
logic pwm1, pwm2, pwm1o, pwm2o;
logic cond_1, cond_2;
logic pwm;

assign PWM_OUT = pwm;

always_ff @(posedge CLK) begin
    t <= {1'b0, TIME};
    C <= {2'b00, CYCLE};
    P <= {1'b0, CYCLE - PHASE_DELAY};
    DL <= {2'b00, DUTY[WIDTH-1:1]};
    DR <= {2'b00, DUTY[WIDTH-1:1]} + {16'h0000, DUTY[0]};

    left <= {1'b0, t} + {1'b0, DL};
    right <= {1'b0, P} + {1'b0, DR};
    t2 <= {1'b0, t};
    p2 <= {1'b0, P};
    dl2 <= {1'b0, DL};
    pc <= C + {1'b0, P};
    tc <= C + {1'b0, t};
    c2 <= C;

    pwm1 <= p2 <= left;
    pwm2 <= t2 < right;
    pwm1o <= pc <= left;
    pwm2o <= tc < right;
    cond_1 <= p2 < dl2;
    cond_2 <= c2 < right;

    pwm <= (pwm1 & pwm2) | ((pwm2 | pwm1o) & cond_1) | ((pwm2o | pwm1) & cond_2);
end

endmodule
