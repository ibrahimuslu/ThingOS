
Cihaz çalışma sistemi
Device Operating System
ThingOS
queue if condition then do sth in mode
condition = could be a ( sensor value) or a (state of a device)
queue = the order in the mode
mode = devices operating modes
do sth = operation state change or send a value to the information channel
CONFIG+C*0#Q*0#I*E#S*1023#F*3#D*D#W*HIGH#T*7
C*(value)#Q*(value)#IF*(value)#STATE*(condition)#FROM*(input)#DO*(value)#WHAT*#TO*(output) // due to some reason not used -> #IN*(mode)
C*1#Q*0#I*E#S*1#F*3#D*D#W*HIGH#T*7 // kısa modu
C = Count
R = Remove from from mode with Queue of mode
Q = queue
C&Q is completly independent order of running mode and running operation respectively so be careful when you are setting Q (order of running operation) to not replace former operation that you set
COND = condition : equals(E) , greater(G) , lower(L), command (C)
E : for digital read
G & L : for analog read
C : only for output without any condition
FROM = input: sensor (A1),device state(D)
DO = kind of device : output pin to device (D), information channel (serial output) (S),RS485(RS)
TO = output: pin number
B = bindedTo: add prior condition , if prior is true dont do anything if false do current
// canceled not used IN = mode:
Should be compiled in Windows
IMPORTANT : !!!! : if an operation changed in HIGH output it stays as HIGH. carefoul while programmingxctu
while(1)
for i< C
if current_mode[i][Q][COND] = "E"
if digitalRead -> current_mode[i][Q][IF] = current_mode[i][Q][FROM]
if current_mode[i][Q][DO] = D
digitalWrite -> current_mode[i][Q][DO],current_mode[i][Q][TO]
if current_mode[i][Q][DO] = S
send current_mode[i][Q][TO]-> current_mode[i][Q][WHAT]
if current_mode[i][Q][DO] = RS
rs485 current_mode[i][Q][TO] -> current_mode[i][Q][WHAT]
if current_mode[i][Q][COND] = "G"
if digitalRead -> current_mode[i][Q][IF] > current_mode[i][Q][FROM]
if current_mode[i][Q][DO] = D
digitalWrite -> current_mode[i][Q][DO],current_mode[i][Q][TO]
if current_mode[i][Q][DO] = S
send current_mode[i][Q][TO]-> current_mode[i][Q][WHAT]
if current_mode[i][Q][DO] = RS
rs485 current_mode[i][Q][TO] -> current_mode[i][Q][WHAT]
if current_mode[i][Q][COND] = "L"
if digitalRead -> current_mode[i][Q][IF] < current_mode[i][Q][FROM]
if current_mode[i][Q][DO] = D
digitalWrite -> current_mode[i][Q][DO],current_mode[i][Q][TO]
if current_mode[i][Q][DO] = S
send current_mode[i][Q][TO]-> current_mode[i][Q][WHAT]
if current_mode[i][Q][DO] = RS
rs485 current_mode[i][Q][TO] -> current_mode[i][Q][WHAT]
if current_mode[i][Q][COND] = "C"
if current_mode[i][Q][DO] = D
digitalWrite -> current_mode[i][Q][DO],current_mode[i][Q][TO]
if current_mode[i][Q][DO] = S
send current_mode[i][Q][TO]-> current_mode[i][Q][WHAT]
if current_mode[i][Q][DO] = RS
rs485 current_mode[i][Q][TO] -> current_mode[i][Q][WHAT]
 do five minutes delay for a time = millis() > 5 dk geçince reverse it.
serialize
split -> # -> current_mode
while current_mode.length
split -> * -> current_mode[i]
removing an operation from mode is not means deleting operation but only removing from queue from runningMode
there is 3 flows in middleware
save the current mode and send to run
get the current running mode number
schedule a mode ( send RUN+{mode_number} at a scheduled time )
discover modules and save them
create device modes with actions
evaporatif modes
fan modu
fan on
cool modu
fan on
sirkülasyon pompası
drenaj
drenaj
3 saatte 1 = 2 dk
hijyen mod
Ntc sıcaklık sensörü
And/or bağlı durumu