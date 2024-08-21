#include <string.h>

enum STATUS {
  STATUS_IDLE = 0,
  STATUS_PROC,
};

STATUS systemStatus;

const int MAX_DATA_SIZE = 32;  // Only allow 32 bytes of payload 
// Arbitrarily chose 32B, though there is probably an upper limit regarding 
// data fidelity or communication method 

const char MSG_SOF = '<';  // Indicates beginning of message
const char MSG_SEP = '|';  // Character to separate command from its data
const char MSG_EOF = '>';  // Indicates end of a message

enum CMD_TYPE {
  CMD_UNKN = 0,  // UNKNOWN
  CMD_STAT,      // GET STATUS
  CMD_S_MS,      // SET MOTOR SPEED
  CMD_G_MS,      // GET MOTOR SPEED
  CMD_S_TP,      // SET TOOL PARAMS
  CMD_G_TP,      // GET TOOL PARAMS
  CMD_HOME,      // HOME AXIS
  CMD_MVTO,      // MOVE TO POSITION
};

enum RESPONSE_TYPE {
  RESP_ACK = 0,  // OK
  RESP_NACK,     // ERROR
  RESP_INFO,     // INFO
};

struct MessagePayload {
  CMD_TYPE cmdType;
  char cmdData[MAX_DATA_SIZE];
};

CMD_TYPE getCommandType(const char* cmdType_s) {
  if (strcmp(cmdType_s, "STAT") == 0) {
    return CMD_STAT;
  } else if (strcmp(cmdType_s, "S_MS") == 0) {
    return CMD_S_MS;
  } else if (strcmp(cmdType_s, "G_MS") == 0) {
    return CMD_G_MS;
  } else if (strcmp(cmdType_s, "S_TP") == 0) {
    return CMD_S_TP;
  } else if (strcmp(cmdType_s, "G_TP") == 0) {
    return CMD_G_TP;
  } else if (strcmp(cmdType_s, "HOME") == 0) {
    return CMD_HOME;
  } else if (strcmp(cmdType_s, "MVTO") == 0) {
    return CMD_MVTO;
  } else {
    return CMD_UNKN;
  }
}

MessagePayload consumeMessage(char* messageIn) {
  MessagePayload payload;

  // Ensure payload is empty
  memset(&payload.cmdType, 0, sizeof(payload.cmdType));
  memset(&payload.cmdData, 0, sizeof(payload.cmdData));

  // Check if the message wrapped in MSG_SOF/MSG_EOF
  if (messageIn[0] == MSG_SOF && messageIn[strlen(messageIn) - 1] == MSG_EOF) {
    // Remove the start and end delimiters
    messageIn++;
    messageIn[strlen(messageIn) - 1] = '\0';

    char* message_consumed[strlen(messageIn)];

    // Split the message into cmd and data using MSG_SEP
    char* token = strtok(messageIn, &MSG_SEP);
    if (token != NULL) {
      // Set CMD_TYPE in payload
      payload.cmdType = getCommandType(token);

      // Get the remaining part as data for command
      token = strtok(NULL, "");

      if (token != NULL) {
        strncpy(payload.cmdData, token, sizeof(payload.cmdData) - 1);
      }
    }
  } else {
    // Handle invalid message
    payload.cmdType = CMD_UNKN;
    strncpy(payload.cmdData, "Unknown Data", sizeof(payload.cmdData) - 1);
  }

  return payload;
}

int getStatus() {
  if (systemStatus == STATUS_IDLE) {
    return 0;
  } else if (systemStatus == STATUS_PROC) {
    return 1;
  }
}

void setStatus(STATUS new_status) {
  systemStatus = new_status;
}

void dispatchMoveTo(char* data) {
  // data would probably have something like:
  // x50y25
  // for now we'll just twiddle systemStatus
  setStatus(STATUS_PROC);
}

char handleCommand(MessagePayload payload) {
  // Arduino only has one thread. So a scheduling schema would probably be 
  // appropriate, but I'm not implementing that now for this toy example.
  char msgOut[MAX_DATA_SIZE];
  // Handle the command based on the type
  switch (payload.cmdType) {
    case CMD_STAT:
      strcpy(msgOut, ("STAT: " + getStatus()));
      break;
    case CMD_MVTO:
      dispatchMoveTo(payload.cmdData);
      break;
  }
  return msgOut;
}

void setup() {
  Serial.begin(115200);
  // wait for serial port to connect. Needed for native USB port only
  while (!Serial) {
    ;
  }
  setStatus(STATUS_IDLE);
}

void loop() {
  Serial.print("System status: ");
  Serial.println(systemStatus);
  if (Serial.available() > 0) {
    String msgIn = Serial.readString();

    // Intentionally not accounting for Null character,
    // will be passing as raw data to consumeMessage
    int messageLength = msgIn.length();
    char message[messageLength];
    msgIn.toCharArray(message, messageLength);

    MessagePayload result = consumeMessage(message);

    char msgOut = handleCommand(result);
    Serial.print("Sending back: ");
    Serial.println(msgOut);
  }
  // delay() is blocking, so this is a pretty negligent implementation.
  // going for simplicity...
  delay(1000);
}
