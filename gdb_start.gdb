# Setup
file firmware.elf
target remote localhost:3333
monitor reset init
set pagination off

define sf
    while (1)
        step
    end
end

# Break at main and go past breakpoint
break main
continue