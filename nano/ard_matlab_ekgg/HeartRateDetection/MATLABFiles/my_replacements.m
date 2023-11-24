function R = my_replacements(lib)


% Copyright 2018 The MathWorks, Inc.
    switch lib
        case 'hand_written'
            R = Replacements_hand_written;
        case 'dst'
            R = Replacements_DSP_System_Toolbox;
        case 'hong'
            R = Replacements_hong;
    end
end