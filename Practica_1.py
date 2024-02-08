"""




"""


import zlib
from Crypto.Cipher import AES
from scapy.all import *

###############################################################################
#                                   CONFIG
###############################################################################
# AES 128 Encryption Key and Initialization Vector
# See this Wikipedia's wiki on Block Cipher for details
# https://en.wikipedia.org/wiki/Block_cipher_mode_of_operation
# 
key = b'\x01\x02\x03\x04\x05\x06\x07\x08\x09\x00\x01\x02\x03\x04\x05\x06'
iv = b'\x01\x02\x03\x04\x05\x06\x07\x08\x09\x00\x01\x02\x03\x04\x05\x06'

verbose = True # True -> Enables debug messages

###############################################################################
#                              INTERNAL METHODS
###############################################################################
# AES128 Encryption function
def enc(message):
    # To encrypt an array its lenght must be a multiple of 16 so we add zeros
    ptxt = message + (b'\x00' * (16 - len(message) % 16))
    # Encrypt in EBC mode
    encryptor = AES.new(key, AES.MODE_CBC, IV=iv)
    ciphed_msg = encryptor.encrypt(ptxt)
    return ciphed_msg

# AES128 Decryption function
def dec(message):
    # Decrypt in EBC mode
    decryptor =  AES.new(key, AES.MODE_CBC, IV=iv)
    deciphed_msg = decryptor.decrypt(message)
    # Remove the Padding
    deciphed_msg = deciphed_msg.rstrip(b'\x00')
    return deciphed_msg

def getSizeWithoutPadding(data):
    # Get size of array
    data_len = len(data)
    lenWoPad = data_len
    for i in range(1,data_len):
        if(data[data_len - i] != 0):
            break
        lenWoPad -= 1
    return lenWoPad

# Computes the CRC32 on the given data
def computeCRC32(data):
    return zlib.crc32(data)

# Extract the CRC value from a frame
def extractCRC32(data, len):
    return (data[len-4] << 24) + (data[len-3] << 16) + (data[len-2] << 8) + data[len-1]


def is_k64_frame(frame):
    try:
        return frame.getlayer(0).src=='d4:be:d9:45:22:61'
        #return frame[802.3].src == 'd4:be:d9:45:22:61'
    except:
        return False

def sendMessage(data):
    # Convert to bytes
    byte_message = bytes(data, 'utf-8')
    
    # Get data len
    data_len = len(byte_message)

    # Add padding for encryption
    if(data_len % 16 != 0):
        padding = bytes(data_len % 16)
        byte_message += padding

    # Encrypt the messafe
    encrypted_message = enc(byte_message)
    encrypted_len = len(encrypted_message)

    # Add CRC32 validation
    dataCRC32 = computeCRC32(encrypted_message)
    bytesCRC32 = dataCRC32.to_bytes(4, byteorder='big')
    encrypted_message += bytesCRC32
    encrypted_len = len(encrypted_message)

    # Add padding
    if(encrypted_len < 46):
        padding = bytes(46-encrypted_len)
        encrypted_message += padding
    
    print("Final len: ", len(encrypted_message))

    # Create frame
    frame = Ether()/encrypted_message

    # Set frame header
    frame[Ether].dst = 'd4:be:d9:45:22:61'
    frame[Ether].src = '58:11:22:86:21:e8'
    frame[Ether].type = data_len

    # Send frame
    sendp(frame)

def main():

    cont = 0

    # Configure Scapy to use the ethernet interface
    conf.iface="Realtek PCIe GbE Family Controller"

    # Wait to receive a frame
    frames = sniff(count=8, lfilter=is_k64_frame) #, prn= lambda x:x.show())

    # Process the frames
    for  i in range(8):
        frame = frames[i]
        data_len = frame.getlayer(0).len
        # Extract data from frame
        data = bytes(frame)

        # Extract the CRC32
        frameSize = getSizeWithoutPadding(data)
        dataCRC32 = extractCRC32(data, frameSize)

        # Compute CRC32 on the given data
        localCRC32 = computeCRC32(data[14:frameSize-4])

        # Exit if CRC32 is incorrect
        if(dataCRC32 != localCRC32):
            print("CRC32 verification is incorrect")
        else:

            decrypted_msg = dec(data[14:frameSize-4])
            print(decrypted_msg)


    # Send test messages
    sendMessage('Hello world')
    sendMessage('Hola a todos!')
    sendMessage('Hugo tuvo un tubo, pero el tubo que tuvo se le rompio. Para recuperar el tubo que tuvo, tuvo que comprar un tubo igual al tubo que tuvo.')


if __name__=="__main__":
    main()