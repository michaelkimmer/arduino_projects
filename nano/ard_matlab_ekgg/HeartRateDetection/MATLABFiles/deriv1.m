function y = deriv1(x0, init)
%  deriv1 implements the derivative approximation represented by
%  the difference equation:
%
%	y[n] = 2*x[n] + x[n-1] - x[n-3] - 2*x[n-4]
%
%  The filter has a delay of 2.
% Copyright 2018 The MathWorks, Inc.
    coder.inline('never');
    persistent x1 x2 x3 x4
    if isempty(x1) || init
        x1 = 0;
        x2 = 0;
        x3 = 0;
        x4 = 0;
    end
    y = 2*x0 + x1 - x3 - 2*x4;
    x4 = x3;
    x3 = x2;
    x2 = x1;
    x1 = x0;
 end