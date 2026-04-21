import message_info
import signal_info

dbc_file_path = r'C:\Projects\Aravindh_Training\Canoe_related\initial_build_VEHCAN.dbc'

#open dbc file and parse line by line

dbc_fh = open(dbc_file_path,'rt')

message_start_key_phrase = 'BO_'
signal_start_key_phrase = 'SG_'

message_list = []

def process_message(input_data):

    global message_list

    #create 1 object for message class
    msg_obj = message_info.message_cl()
    msg_obj.id = int(input_data[1], 10)
    msg_obj.name = input_data[2][0: len(input_data[2])-1]
    msg_obj.dlc = int(input_data[3],10)
    msg_obj.tx_node = input_data[4]
    # push local obj variable into global message_list
    message_list.append(msg_obj)


def process_signal(input_data):
    global message_list

    # create signal object, fill data and push it into the last message object that was created
    l_signal_obj = signal_info.signal_cl()
    l_signal_obj.name = input_data[1]

    message_list[-1].list_of_signals.append(l_signal_obj)


for one_line in dbc_fh.readlines():
    if one_line.find(message_start_key_phrase) != -1:
        one_line = one_line.replace('\n', '')
        one_line = one_line.replace('\t', '')

        split_output = one_line.split(' ')
        # if size of split_output > 1 and 0th element is BO_, then process message
        if len(split_output)> 1 and split_output[0] == message_start_key_phrase:
            process_message(split_output)

    elif one_line.find(signal_start_key_phrase) != -1:
        one_line = one_line.replace('\n', '')
        split_output = one_line.split(' ')
        if len(split_output) > 1:
            # remove the empty elements from the string
            i = len(split_output) - 1
            while(  i >= 0 ):
                if split_output[i] == '':
                    del split_output[i]

                i = i-1

            if split_output[0] == signal_start_key_phrase:
                process_signal(split_output)


m = 0

