void debug_logMessage(struct MessageHeader message, struct SerializeHeader serial) {
	fprintf(stderr, "\tmessage.type=%s\n", reflect(MessageType, message.type));
	fprintf(stderr, "\tmessage.protocolVersion=%u\n", message.protocolVersion);
	fprintf(stderr, "\tserial.length=%u\n", serial.length);
	if(message.type == MessageType_UserMessage)
		fprintf(stderr, "\tserial.type=%s\n", reflect(UserMessageType, serial.type));
	else if(message.type == MessageType_DedicatedServerMessage)
		fprintf(stderr, "\tserial.type=%s\n", reflect(DedicatedServerMessageType, serial.type));
	else if(message.type == MessageType_HandshakeMessage)
		fprintf(stderr, "\tserial.type=%s\n", reflect(HandshakeMessageType, serial.type));
}

void debug_logType(struct MessageHeader message, struct SerializeHeader serial) {
	if(message.type == MessageType_UserMessage)
		fprintf(stderr, "recieve UserMessageType_%s\n", reflect(UserMessageType, serial.type));
	else if(message.type == MessageType_DedicatedServerMessage)
		fprintf(stderr, "recieve DedicatedServerMessageType_%s\n", reflect(DedicatedServerMessageType, serial.type));
	else if(message.type == MessageType_HandshakeMessage)
		fprintf(stderr, "recieve HandshakeMessageType_%s\n", reflect(HandshakeMessageType, serial.type));
}
