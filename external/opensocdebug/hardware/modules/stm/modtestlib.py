import cocotb
from cocotb.triggers import Timer
from cocotb.clock import Clock
from cocotb.result import TestFailure
from cocotb.triggers import RisingEdge

CONST_PACKET_TYPE_REG                          = 0b00
CONST_PACKET_TYPE_PLAIN                        = 0b01
CONST_PACKET_TYPE_EVENT                        = 0b10

CONST_PACKET_SUBTYPE_REQ_READ_REG_16           = 0b0000
CONST_PACKET_SUBTYPE_REQ_READ_REG_32           = 0b0001
CONST_PACKET_SUBTYPE_REQ_READ_REG_64           = 0b0010
CONST_PACKET_SUBTYPE_REQ_READ_REG_128          = 0b0011
CONST_PACKET_SUBTYPE_RESP_READ_REG_SUCCESS_16  = 0b1000
CONST_PACKET_SUBTYPE_RESP_READ_REG_SUCCESS_32  = 0b1001
CONST_PACKET_SUBTYPE_RESP_READ_REG_SUCCESS_64  = 0b1010
CONST_PACKET_SUBTYPE_RESP_READ_REG_SUCCESS_128 = 0b1011
CONST_PACKET_SUBTYPE_RESP_READ_REG_ERROR       = 0b1100
CONST_PACKET_SUBTYPE_REQ_WRITE_REG_16          = 0b0100
CONST_PACKET_SUBTYPE_REQ_WRITE_REG_32          = 0b0101
CONST_PACKET_SUBTYPE_REQ_WRITE_REG_64          = 0b0110
CONST_PACKET_SUBTYPE_REQ_WRITE_REG_128         = 0b0111
CONST_PACKET_SUBTYPE_REQ_WRITE_REG_SUCCESS     = 0b1110
CONST_PACKET_SUBTYPE_REQ_WRITE_REG_ERROR       = 0b1111

CONST_BASE_REG_TEST                            = 0

@cocotb.coroutine
def build_flit(dut, data, last):
    """function which transmits one flit between debug modules

    Args:
        dut:            device under test.
        data:           16 bit word which is to be transferred.
        last:           boolean value indicating if it is the last word of a packet.

    Returns:
        -

    """
    
    # set the signals  
    dut.debug_in.data <= data
    dut.debug_in.valid <= 1
    dut.debug_in.last <= last
     
    # wait for the active clock edge
    yield RisingEdge(dut.clk)
    # wait for the module to be ready to accept a data word
    while not dut.debug_in_ready.value:
         yield RisingEdge(dut.clk)
    dut.debug_in.valid <= 0
    
    
@cocotb.coroutine
def receive_packet(dut, response):
    """successively fills a buffer with incoming data words until the last flit of a packed is received

    Args:
        dut:            device under test.
        response:       (preferably) empty buffer which the incoming flits are stored in.

    Returns:
        response of the debug module of variable length in the 'response'-buffer

    """
    ## GET RESPONSE
    while True:
        # wait for the active clock edge
        yield RisingEdge(dut.clk)
        if dut.debug_out.valid.value:
            # extend the buffer with the debug module's output if it indicates it to be valid
            response.append(dut.debug_out.data.value.integer)
        
            if dut.debug_out.last.value:
                # stop with reading in the response if the last flit has been sent
                break
            
    
@cocotb.coroutine
def evaluate_packet(dut, response, destination, source, width, expectedPL):
    """compares a received packet with its expectation values and outputs the result in the console

    Args:
        dut:            device under test.
        response:       read in response of the debug module.
        destination:    id of the module which initially was the destination.
        source:         id of the module which initially was the source.
        width:          data width of the register the data was read from/written to.
        expectedPL:     expected received payload.

    Returns:
        -

    """
    result = 1
    
    #
    if response[0] != source:
        # test failed if the response was intended for a different module
        raise TestFailure("Expected dest to be 0x%x, got 0x%x" % (source, response[0]))
        result = 0
    #
    if response[1] != destination:
        # test failed if the response was not sent from the target module
        raise TestFailure("Expected src to be 0x%x, got 0x%x" % (destination, response[1]))
        result = 0
    #
    for x in range(0, (width/16)):
        # step through the individual payload  flits
        if response[3 + x] != expectedPL[x]:
            # test failed if a payload flit does not match its expectation value
            raise TestFailure("Expected payload word %d to be 0x%x, got 0x%x" % (x, expectedPL[x], response[3 + x]))
            result = 0
    
    if result:
        # test was successful if none of the above mentioned errors occurred
        dut._log.info("Passed the test!")
        
        # clear the buffer for the next RegRead()
    del response[:]
        
    for _ in range(1):
        yield RisingEdge(dut.clk)
        
    
@cocotb.coroutine    
def send_packet(dut, destination, source, packet_type, packet_subtype, message):
    """function which takes necessary information as its arguments and transmits a complete packet to a chosen debug module

    Args:
        dut:            device under test.
        destination:    id of the module which is the destination.
        source:         id of the module which is the source.
        packet_type:    possible values are: CONST_PACKET_TYPE_REG, CONST_PACKET_TYPE_PLAIN and CONST_PACKET_TYPE_EVENT.
        packet_subtype: choose from one of the const values: read or write requests of variable length as well as read/write responses(success/error).
        message:        a payload of variable length (up to 8 16 bit words)

    Returns:
        -

    """
    
    # build the 3 header flits in advance
    dest = constructheader1(destination)
    sour = constructheader2(source)
    type = constructheader3(packet_type, packet_subtype)
    
    # transmit the 3 header flits
    yield build_flit(dut, dest,    0)
    yield build_flit(dut, sour,    0)
    yield build_flit(dut, type,    0)
    
    # number of single messages
    # has to be changed since messages of multiple words are assumed to be lists of single words
    y = (message+65536)/65536
    
    if y > 8:
        # output an error if too many flits are to be sent
        raise TestFailure("Number of data words exceeds upper limit!")
    
    for x in range(0, y):
        # extract one word after the other by masking the entire message with a shifted bit window
        goal = 0xFFFF << (65536 * x)
        newmessage = (message & goal) >> (65536 * x)
        # determine if the last flit has to be sent and indicate this by a set 'last'-bit
        if x == (y-1):
            yield build_flit(dut, newmessage, 1)
        else:
            yield build_flit(dut, newmessage, 0)

        
@cocotb.coroutine    
def init(dut, ID):
    """function which initializes the testbench by setting up the clock, resetting the device under test and assigning an ID to it
        
        Args:
        dut:            device under test.
        ID:             module address which is assigned to the dut.
        
        Returns:
        -
        
    """
    
    # setup clock
    cocotb.fork(Clock(dut.clk, 1000).start())
    
    # reset
    dut._log.info("Resetting DUT")
    dut.rst <= 1

    dut.trace_valid <= 1
    dut.debug_out_ready <= 1
    dut.debug_in.valid <= 1
    
    # set module address to the second function parameter
    dut.id <= ID

    for _ in range(2):
        yield RisingEdge(dut.clk)
    dut.rst <= 0
    

 
def constructheader1(destination):
    """function which returns a data word which can later on be used for the transmission of the first packet header
        
        Args:
        destination:    id of the module which is the destination.           .
        
        Returns:
                        16 bit word containing the information which will be transmitted by the first packet header.
        
    """
    
    return 0x0000 | (destination & 0x03FF)
    
     
def constructheader2(source):
    """function which returns a data word which can later on be used for the transmission of the second packet header
        
        Args:
        source:         id of the module which is the source.          .
        
        Returns:
                        16 bit word containing the information which will be transmitted by the second packet header.
        
    """

    return 0x0000 | (source & 0x03FF)
    
    
def constructheader3(packet_type, packet_subtype):
    """function which returns a data word which can later on be used for the transmission of the third packet header
        
        Args:
        packet_type:    possible values are: CONST_PACKET_TYPE_REG, CONST_PACKET_TYPE_PLAIN and CONST_PACKET_TYPE_EVENT.
        packet_subtype: choose from one of the const values: read or write requests of variable length as well as read/write responses(success/error).
        
        Returns:
                        16 bit word containing the type of the data request for a packet.
        
    """

    return 0x0000 | (packet_type << 14) | (packet_subtype << 10)
 
@cocotb.coroutine        
def RegWrite(dut, destination, source, reg_width, address, content, response):
    """function which uses the subroutine 'send_packet' to write a new value into a specified register and reads in the response to tell the user if the write process was successful
        
        Args:
        dut:            device under test.
        destination:    id of the module which is the destination.
        source:         id of the module which is the source.
        reg_width:      choose between 16, 32, 64 and 128 bit register access.
        address:        address of the register the new value is to be written to.
        content:        value which will be written to the specified register
        response:       buffer variable which the response of the dut will be stored in
        
        Returns:
        -
        
    """
    
    # send a write request to the dut depending on how many data words are to be written into the memory
    if reg_width == 16:
        yield send_packet(dut, destination, source, CONST_PACKET_TYPE_REG, CONST_PACKET_SUBTYPE_REQ_WRITE_REG_16,  content)
    elif reg_width == 32:
        yield send_packet(dut, destination, source, CONST_PACKET_TYPE_REG, CONST_PACKET_SUBTYPE_REQ_WRITE_REG_32,  content)
    elif reg_width == 64:
        yield send_packet(dut, destination, source, CONST_PACKET_TYPE_REG, CONST_PACKET_SUBTYPE_REQ_WRITE_REG_64,  content)
    elif reg_width == 128:
        yield send_packet(dut, destination, source, CONST_PACKET_TYPE_REG, CONST_PACKET_SUBTYPE_REQ_WRITE_REG_128, content)
    else:
        # display an error message if an unsupported register width was chosen
        raise TestFailure("Invalid register width parameter")

    # read in the response of the debug module
    response = []
    yield receive_packet(dut, response)
    
    # if the module returned a success, everything is fine
    if ((response[2] & 0x3C00) >> 10) == CONST_PACKET_SUBTYPE_REQ_WRITE_REG_SUCCESS:
        print("Write process was successful!")
    # if it returned an error, output that error
    elif ((response[2] & 0x3C00) >> 10) == CONST_PACKET_SUBTYPE_REQ_WRITE_REG_ERROR:
        print("An error occurred during the write process!")
    # otherwise something else went wrong
    else:
        print("An unidentified error occurred!")
    
@cocotb.coroutine        
def RegRead(dut, destination, source, reg_width, address, response):
    """function which uses the subroutine 'send_packet' to read a value from a specified register and reads in the response to tell the user if the read process was successful
        
        Args:
        dut:            device under test.
        destination:    id of the module which is the destination.
        source:         id of the module which is the source.
        reg_width:      choose between 16, 32, 64 and 128 bit register access.
        address:        address of the register the value is to be read from.
        response:       buffer variable which the response of the dut will be stored in
        
        Returns:
        -
        
    """
    
    # branch depending on the width of the accessed
    if reg_width == 16:
        # transmit the read request
        yield send_packet(dut, destination, source, CONST_PACKET_TYPE_REG, CONST_PACKET_SUBTYPE_REQ_READ_REG_16,  address)
        
        # read in the response
        yield receive_packet(dut, response)
        
        # print the response of the dut
        print("Answer of the dut:")
        print '[{}]'.format(', '.join(hex(x) for x in response))
        
        # if the module returned a success, everything is fine
        if ((response[2] & 0x3C00) >> 10) == CONST_PACKET_SUBTYPE_RESP_READ_REG_SUCCESS_16:
            print("Read process was successful!")
        # if it returned an error, output that error
        elif ((response[2] & 0x3C00) >> 10) == CONST_PACKET_SUBTYPE_RESP_READ_REG_ERROR:
            print("An error occurred during the read process!")
        # otherwise something else went wrong
        else:
            print("An unidentified error occurred!")
            
    elif reg_width == 32:
        # transmit the read request
        yield send_packet(dut, destination, source, CONST_PACKET_TYPE_REG, CONST_PACKET_SUBTYPE_REQ_READ_REG_32,  address)
        
        # read in the response
        yield receive_packet(dut, response)
        
        # if the module returned a success, everything is fine
        if ((response[2] & 0x3C00) >> 10) == CONST_PACKET_SUBTYPE_RESP_READ_REG_SUCCESS_32:
            print("Read process was successful!")
        # if it returned an error, output that error
        elif ((response[2] & 0x3C00) >> 10) == CONST_PACKET_SUBTYPE_RESP_READ_REG_ERROR:
            print("An error occurred during the read process!")
        # otherwise something else went wrong
        else:
            print("An unidentified error occurred!")
            
    elif reg_width == 64:
        # transmit the read request
        yield end_packet(dut, destination, source, CONST_PACKET_TYPE_REG, CONST_PACKET_SUBTYPE_REQ_READ_REG_64,  address)
        
        # read in the response
        yield receive_packet(dut, response)
        
        # if the module returned a success, everything is fine
        if ((response[2] & 0x3C00) >> 10) == CONST_PACKET_SUBTYPE_RESP_READ_REG_SUCCESS_64:
            print("Read process was successful!")
        # if it returned an error, output that error
        elif ((response[2] & 0x3C00) >> 10) == CONST_PACKET_SUBTYPE_RESP_READ_REG_ERROR:
            print("An error occurred during the read process!")
        # otherwise something else went wrong
        else:
            print("An unidentified error occurred!")
            
    elif reg_width == 128:
        # transmit the read request
        yield send_packet(dut, destination, source, CONST_PACKET_TYPE_REG, CONST_PACKET_SUBTYPE_REQ_READ_REG_128,  address)
        
        # read in the response
        yield receive_packet(dut, response)
        
        # if the module returned a success, everything is fine
        if ((response[2] & 0x3C00) >> 10) == CONST_PACKET_SUBTYPE_RESP_READ_REG_SUCCESS_128:
            print("Read process was successful!")
        # if it returned an error, output that error
        elif ((response[2] & 0x3C00) >> 10) == CONST_PACKET_SUBTYPE_RESP_READ_REG_ERROR:
            print("An error occurred during the read process!")
        # otherwise something else went wrong
        else:
            print("An unidentified error occurred!")
            
    else:
        # display an error message if an unsupported register width was chosen
        raise TestFailure("Invalid register width parameter")
    
    for _ in range(3):
        yield RisingEdge(dut.clk)

    
@cocotb.coroutine        
def ModuleSubTest_STM(dut, test_id):
    """function which takes a second parameter aside from the device under test to specify which functionality of the debug module should be tested and which tells the user if the correct values are stored inside the base registers of a debug module
        
        Args:
        dut:            device under test.
        test_id:        ID indicating which functionality should be tested.
        
        Returns:
        -
        
    """
    
    # other tests could potentially be added
    if test_id == CONST_BASE_REG_TEST:
            
        response = []
        
        #============================================================================================================Test MOD_ID
        
        # specify how the result should look like (has to be changed to defining a single value instead of a list)
        expected_payload = []
        expected_payload.append(1)
        
        #             dut, destination, source, word width, reg address, list for return value
        yield RegRead(dut, 1,           0,      16,         0,           response)
    
        #                     dut, response, destination(dut), source(simulated), word width, expected payload, success variable
        yield evaluate_packet(dut, response, 1,                0,                 16,         expected_payload)
        
        #=======================================================================================================Test MOD_VERSION
        
        # specify how the result should look like (has to be changed to defining a single value instead of a list)
        expected_payload = []
        expected_payload.append(4)
        
        #             dut, destination, source, word width, reg address, list for return value
        yield RegRead(dut, 1,           0,      16,         1,           response)
    
        #                     dut, response, destination(dut), source(simulated), word width, expected payload, success variable
        yield evaluate_packet(dut, response, 1,                0,                 16,         expected_payload)
                
        #========================================================================================================Test MOD_VENDOR
        
        # specify how the result should look like (has to be changed to defining a single value instead of a list)
        expected_payload = []
        expected_payload.append(0)
        
        #             dut, destination, source, word width, reg address, list for return value
        yield RegRead(dut, 1,           0,      16,         2,           response)
    
        #                     dut, response, destination(dut), source(simulated), word width, expected payload, success variable
        yield evaluate_packet(dut, response, 1,                0,                 16,         expected_payload)
                
        #============================================================================================================Test MOD_CS
        #
        # expected_payload = []
        # expected_payload.append(1)
        #
        #               dut, destination, source, word width, reg address, list for return value
        # yield RegRead(dut, 1,           0,      16,         3,           response)
        #
        #                       dut, response, destination(dut), source(simulated), word width, expected payload, success variable
        # yield evaluate_packet(dut, response, 1,                0,                 16,         expected_payload)
        #
        #====================================================================================================Test MOD_EVENT_DEST
        #
        # expected_payload = []
        # expected_payload.append(1)
        #
        #               dut, destination, source, word width, reg address, list for return value
        # yield RegRead(dut, 1,           0,      16,         4,           response)
        #
        #                       dut, response, destination(dut), source(simulated), word width, expected payload, success variable
        # yield evaluate_packet(dut, response, 1,                0,                 16,         expected_payload)
        #
        for _ in range(1):
            yield RisingEdge(dut.clk)
