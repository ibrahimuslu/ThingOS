ThingOS
--Cihaz çalışma sistemi
--Device Operating System

queue if condition then do sth in mode

- condition = could be a ( sensor value) or a (state of a device)
- queue = the order in the mode
- mode = devices operating modes
- do sth = operation state change or send a value to the information channel

CONFIG+C*0#Q*0#I*E#S*1023#F*3#D*D#W*HIGH#T*7

C*(value)#Q*(value)#IF*(value)#STATE*(condition)#FROM*(input)#DO*(value)#WHAT*#TO*(output) // due to some reason not used -&gt; #IN*(mode)

C*1#Q*0#I*E#S*1#F*3#D*D#W*HIGH#T*7 // kısa modu

- C = Count
- R = Remove from from mode with Queue of mode
- Q = queue
- C&Q is completly independent order of running mode and running operation respectively so be careful when you are setting Q (order of running operation) to not replace former operation that you set
- COND = condition : equals(E) , greater(G) , lower(L), command (C)
    - E : for digital read
    - G & L : for analog read
    - C : only for output without any condition

- FROM = input: sensor (A1),device state(D)
- DO = kind of device : output pin to device (D), information channel (serial output) (S),RS485(RS)
- TO = output: pin number
- B = bindedTo: add prior condition , if prior is true dont do anything if false do current
- Should be compiled in Windows
- IMPORTANT : !!!! : if an operation changed in HIGH output it stays as HIGH. carefoul while programmingxctu
