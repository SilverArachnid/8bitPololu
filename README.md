# 8bitPololu
The below files can be executed on the Pololu robot to observe the data transmission protocol
Currently only one of the bots can serve as the transmitter or reciever, a 2-way communication will be added in future versions


3pi_transmitter - the code is to transmit bits randomly 
3pi_reciever - the code is to recieve the transmitted bits and see the plots on the reciver serial monitor(Line sensors)

3pi_transmitter_message - the code is to transmit a message from the transmitter as discussed in the report
3pi_reciever_message - the code is to recieve the transmitted messages and decode them tp print them on the OLED display. This is an initial version of the code that was further improved for the testing (Line Sensors)

3pi_transmitter_message_test_FSM - this is the testing code with an improved algorithm that works with '3pi_transmitter_message' and decodes messages for a fixed time to then print out accuracy and statistical parameters on the serial monitor (Line Sensors)

3pi_transmitter_message_test_Freq - this is the tesing code but to also print out the frequency for the frequency testing part(Line Sensors)

3pi_receiver_IR_basic - this is one of the initial version of the Bump sensor code without any thresholding. It worked only for a short distance.

3pi_receiver_IR_testing - this is the Bump sensor testing code, it can be uses similarly for testing accuracy with 3pi_transmitter_message for sending and recivering messages.

