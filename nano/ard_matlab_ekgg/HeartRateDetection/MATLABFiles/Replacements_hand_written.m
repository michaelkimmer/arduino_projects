classdef Replacements_hand_written
    
    % Copyright 2018 The MathWorks, Inc.
    methods
        
        function y = lpfilt(~,x)
            y = lpfilt(x);
        end
        
        function y = hpfilt(~,x)
            y = hpfilt(x);
        end
        
        function y = deriv1(~,x)
            y = deriv1(x);
        end
        
        function y = mvwint(~,x)
            y = mvwint(x);
        end
        
        function y = peak(~,x)
            y = peak(x);
        end
        
        function [QRSDelay, threshold] = qrs_delay(~,pk)
            [QRSDelay, threshold] = qrs_delay(pk);
        end
        
    end
end
