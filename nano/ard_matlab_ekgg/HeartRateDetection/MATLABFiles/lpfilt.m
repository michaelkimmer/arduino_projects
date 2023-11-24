function y = lpfilt(x)
%  lpfilt() implements the digital filter represented by the difference
%  equation:
%
% 	y[n] = 2*y[n-1] - y[n-2] + x[n] - 2*x[n-5] + x[n-10]
%
%	Note that the filter delay is five samples.
%
%  y(n) = 2*y(n-1) - y(n-2) + x(n) - 2*x(n-5) + x(n-10)
%
%       1*y(n) - 2*y(n-1) + 1*y(n-2) = 1*x(n) + 0*x(n-1) + 0*x(n-2) + 0*x(n-3) +  0*x(n-4) - 2*x(n-5) + 0*x(n-6) + 0*x(n-7) + 0*x(n-8)  + 0*x(n-9) + 1*x(n-10)
%  a = [1       -2          1];   b = [1        0          0          0           0         -2          0          0          0           0          1];
%
% Lowpass filter coefficients.  The roots of a are on the unit circle, but
% they exactly cancel with roots of b.
% Dividing numerator and denominator gives q = [1 2 3 4 5 4 3 2 1].
% It's counting on perfect cancellation in order to get a filter that only
% has powers of two for coefficients.
% b = [1  0   0   0    0  -2   0   0   0    0   1];
% a = [1  -2  1];
% roots_a = roots(a)
% [q,r] = deconv(b,a) %#ok<*NOPTS>
    % Copyright 2018 The MathWorks, Inc.
    coder.inline('never');
    persistent x1 x2 x3 x4 x5 x6 x7 x8 x9 x10
    persistent y1 y2
    y = cast(0,'like',x);
    y0 = cast(0,'like',x);
    if isempty(x1)
        x1 = cast(0,'like',x);
        x2 = cast(0,'like',x);
        x3 = cast(0,'like',x);
        x4 = cast(0,'like',x);
        x5 = cast(0,'like',x);
        x6 = cast(0,'like',x);
        x7 = cast(0,'like',x);
        x8 = cast(0,'like',x);
        x9 = cast(0,'like',x);
        x10 = cast(0,'like',x);
        y1 = cast(0,'like',x);
        y2 = cast(0,'like',x);
    end
    y0(:) = bitsll(y1,1) - y2 + x - bitsll(x5,1) + x10;
    y2(:) = y1;
    y1(:) = y0;

    y(:) = bitsra(y0,5);

    x10 = x9;
    x9  = x8;
    x8  = x7;
    x7  = x6;
    x6  = x5;
    x5  = x4;
    x4  = x3;
    x3  = x2;
    x2  = x1;
    x1  = x;
end
