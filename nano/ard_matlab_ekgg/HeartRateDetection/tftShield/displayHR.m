classdef displayHR < matlab.System & matlab.system.mixin.Propagates ...
         & coder.ExternalDependency & matlab.system.mixin.CustomIcon 
    % displayHR 
    %
    
    %#codegen
    %#ok<*EMCA>
    
    % Public, non-tunable properties
    properties(Nontunable)
        %Text Color
        txtColor_ = 'Black';
        %Text size
        txtSize_  = uint8(5);
        %Text X-Position
        x_        = 0;
        %Text Y-Position
        y_        = 0;
    end

    % Pre-computed constants
    properties(Constant, Hidden)
            txtColor_Set = matlab.system.StringSet({'Black','Navy','Darkgreen','Darkcyan','Maroon','Purple','Olive','Lightgrey','Darkgrey',...
                         'Green','Cyan','Red','Magenta','Yellow','White','Orange','Greenyellow','Pink'});
    end

    properties(Access = private)
        ColorCode = {'Black','Navy','Darkgreen','Darkcyan','Maroon','Purple','Olive','Lightgrey','Darkgrey',...
                         'Green','Cyan','Red','Magenta','Yellow','White','Orange','Greenyellow','Pink'};
        prevHeartRate=uint8(0);
        ColorIndex=uint8(0);
    end

    methods
        % Constructor
        function obj = displayHR(varargin)
            % Support name-value pair arguments when constructing object
            setProperties(obj,nargin,varargin{:})
        end
        
        function set.x_(obj, val)
            validateattributes(val, {'numeric'}, ...
                {'real', 'integer','scalar', ...
                '>=', 0,'<=', 320},'', 'Slave Address');
            obj.x_ = val;
        end
        
        function set.y_(obj, val)
            validateattributes(val, {'numeric'}, ...
                {'real', 'integer','scalar', ...
                '>=', 0,'<=', 240},'', 'Slave Address');
            obj.y_ = val;
        end
    end

    methods(Access = protected)
        %% Common functions
        function setupImpl(obj)
            % Perform one-time calculations, such as computing constants
            if coder.target('Rtw')
                [~,c] = size(obj.ColorCode);
                for i = 1:c
                    if(strcmp(obj.txtColor_,obj.ColorCode{i}))
                        obj.ColorIndex = uint8(i-1);
                    end
                end
                coder.cinclude('heart_monitor.h')
                coder.ceval('tftSetup');
                coder.ceval('setTextSize', obj.txtSize_);
            end
        end

        function stepImpl(obj,HR)
            % Implement algorithm. Calculate y as a function of input u and
            % discrete states.
            if coder.target('Rtw')
                if(obj.prevHeartRate ~= HR)
                    x_pos = uint16(obj.x_);
                    y_pos = uint16(obj.y_);
                    coder.ceval('displayHeartRate',obj.ColorIndex,HR,x_pos,y_pos);
                    obj.prevHeartRate = uint8(HR);
                end
            end
        end
        
        function inputname = getInputNamesImpl(~)
            inputname = 'Heart Rate';
        end
        
        
        function N = getNumInputsImpl(~)
            % Specify number of System inputs
            N = 1;
        end
        
        
        function N = getNumOutputsImpl(~)
            % Specify number of System outputs
            N = 0;
        end
        
        function flag = isInputSizeLockedImpl(~,~)
            flag = true;
        end
        
        function varargout = isInputFixedSizeImpl(~,~)
            varargout{1} = true;
        end
        
        function flag = isInputComplexityLockedImpl(~,~)
            flag = true;
        end
        
        function validateInputsImpl(~, u)
            if coder.target('Rtw')
                % Run input validation 
                classes = {'numeric'};
                attributes = {'real','integer'};
                paramName = 'Heart Rate';
                validateattributes(u,classes,attributes,'',paramName);
            end
        end
        
        function icon = getIconImpl(~)
        % Define a string as the icon for the System block in Simulink.
        icon = 'Display Heart Rate';
        end 
    end

    methods(Static, Access = protected)
        %% Simulink customization functions
        function header = getHeaderImpl
            % Define header panel for System block dialog
            header = matlab.system.display.Header(...
                'ShowSourceLink', false, ...
                'Title', 'Display Heart Rate',... 
                'Text', 'Displays the current heart rate onto the LCD screen');
                
        end

        function simMode = getSimulateUsingImpl
            % Return only allowed simulation mode in System block dialog
            simMode = 'Interpreted execution';
        end
        
        function flag = showSimulateUsingImpl
            % Return false if simulation mode hidden in System block dialog
            flag = false;
        end
        
        
        function groups = getPropertyGroupsImpl
            % Define property section(s) for System block dialog
            TxtColorProp = matlab.system.display.internal.Property(...
                'txtColor_', 'Description', 'Text Color');
            TxtSizeProp = matlab.system.display.internal.Property(...
                'txtSize_', 'Description', 'Text Size');
            XPosProp = matlab.system.display.internal.Property(...
                'x_', 'Description', 'Text X-Position');
            YPosProp = matlab.system.display.internal.Property(...
                'y_', 'Description', 'Text Y-Position');
            
            Group = matlab.system.display.Section(...
                'Title', 'Parameters', 'PropertyList', ...
                {TxtColorProp,TxtSizeProp,XPosProp,YPosProp});
            
            groups = Group;
        end
        
    end
    
    
    methods (Static)
        function name = getDescriptiveName()
             name = 'Display Heart Rate';
        end
        
        function b = isSupportedContext(context)
            b = context.isCodeGenTarget('rtw');
        end
        
        function updateBuildInfo(buildInfo, context)
            % Update the build-time buildInfo
            if context.isCodeGenTarget('rtw')
                  spiRootDir = codertarget.arduinobase.internal.getArduinoIDERoot;
                  % Include Paths
                  addIncludePaths(buildInfo, fullfile(spiRootDir, 'hardware\arduino\avr\libraries\SPI\src'));
                  spkgRootDir = fileparts(strtok(mfilename('fullpath'), '+'));
                  % Include Paths
                  addIncludePaths(buildInfo, fullfile(spkgRootDir, 'include'));
                  addIncludeFiles(buildInfo, 'heart_monitor.h');
                  % Source Files
                  systemTargetFile = get_param(buildInfo.ModelName,'SystemTargetFile');
                  if isequal(systemTargetFile,'ert.tlc')
                    % Add the following when not in rapid-accel simulation
                    addSourcePaths(buildInfo, fullfile(spkgRootDir, 'include'));
                    addSourcePaths(buildInfo, fullfile(spiRootDir, 'hardware\arduino\avr\libraries\SPI\src'));
                    addSourceFiles(buildInfo, 'SPI.cpp', fullfile(spiRootDir, 'hardware\arduino\avr\libraries\SPI\src'), 'BlockModules');
                    addSourceFiles(buildInfo, 'Adafruit_ILI9341.cpp', fullfile(spkgRootDir, 'include'), 'BlockModules');
                    addSourceFiles(buildInfo, 'Adafruit_SPITFT.cpp', fullfile(spkgRootDir, 'include'), 'BlockModules');
                    addSourceFiles(buildInfo, 'Adafruit_GFX.cpp', fullfile(spkgRootDir, 'include'), 'BlockModules');
                    addSourceFiles(buildInfo, 'heart_monitor.cpp', fullfile(spkgRootDir, 'include'), 'BlockModules');
                  end     
            end           
        end
    end
end
