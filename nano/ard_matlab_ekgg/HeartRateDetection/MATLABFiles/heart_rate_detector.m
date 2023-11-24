function [heart_rate, lp, hp, avg, pk, threshold, QRSDelay] = heart_rate_detector(u, T, replacement_lib)
% heart_rate_detector QRS detection based on heart_rate_detector in picqrs.c from
% Open Source ECG Analysis (OSEA)
% http://www.eplimited.com/confirmation.htm
% Copyright 2018 The MathWorks, Inc.
    coder.inline('never');
    R = my_replacements(replacement_lib);
    x = cast(u, 'like', T.data);
    heart_rate = cast(0,'like',T.heart_rate_output);
    lp = R.lpfilt(x);
    hp = R.hpfilt(lp);
    d = R.deriv1(hp);
    a = abs(d);
    avg = R.mvwint(a);
    pk = R.peak(avg);

    [QRSDelay, threshold] = R.qrs_delay(pk);
    if QRSDelay ~= 0
        % (1000 ms/s * 60s/m) / (samples per ms) / (QRSDelay samples)
        heart_rate(:) = cast(60000 / SAMPLES_PER_MS,'like',T.int) / QRSDelay;
    end
end
