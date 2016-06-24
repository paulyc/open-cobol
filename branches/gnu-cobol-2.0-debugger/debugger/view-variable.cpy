      * view-variable.cpy
      * Codebestandteile, die explizit für die Variablenanzeige (und Manipulation) bestimmt sind
      
      ***************************************************************
        view-variable section.
            move 3 to tmp-unstring-ptr.
            move spaces to tmp-unstring-buffer.
            unstring tmp-command-input-buffer delimited by space 
                into tmp-unstring-buffer
                with pointer tmp-unstring-ptr
            end-unstring
            
            move low-value to tmp-unstring-buffer(30:1).
      *      if cob-anim-logging = 'Y'
      *          move "After unstring..." to tmp-log-line
      *          perform log-msg
      *          move "After low-value..." to tmp-log-line
      *          perform log-msg
      *          move tmp-unstring-buffer to tmp-log-line
      *          perform log-msg
      *          move "After disp..." to tmp-log-line
      *          perform log-msg
      *          perform log-interface-block
      *          move "Before call animdata..." to tmp-log-line
      *          perform log-msg
      *          string "Calling " delimited by size 
      *              ANIDATA delimited by spaces
      *              into tmp-log-line
      *          end-string 
      *          perform log-msg
      *      end-if
                        
            call ANIDATA using "G" interface-block 
            tmp-unstring-buffer end-call
            
            display empty-screen end-display
            
            move tmp-unstring-buffer to name-vv-content.
            move dtf-length to size-vv-content.
            move dtf-usage to usage-vv-content.
            move dtf-value(1:60) to content-vv-l1.
            move dtf-value(61:60) to content-vv-l2.
            move dtf-value(121:60) to content-vv-l3.
            move dtf-value(181:60) to content-vv-l4.
            move dtf-value(241:40) to content-vv-l5.
            display view-variable-screen end-display
            
            move 0 to dtf-length, dtf-usage.
            move spaces to dtf-value.
            
            move "Back" to func1.
            move spaces to func2, func3, func4, func5.
            display footline-2 end-display
            display footline end-display
            
            move "After calling animdata..." to tmp-log-line.
            perform log-msg.
            perform log-interface-block.
            
            accept tmp-command-input-buffer line 23 col 10 end-accept

            perform process-function-key-vv
      *          perform process-input-buffer
            continue.
            
      **************************************************************
        goback-from-vv section.
            
            display empty-screen end-display
            perform do-screen-init.
            display animator-screen end-display
            perform display-current-breakpoints.
            perform display-active-line.
            
            continue.
