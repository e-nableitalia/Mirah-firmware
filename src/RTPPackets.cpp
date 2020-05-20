
#include <RTPPackets.hpp>

// first byte
#define RTP_VER_MASK		0xc0
#define RTP_PADDING_MASK	0x20
#define RTP_EXT_MASK		0x10
#define RTP_CC_MASK			0x0f

#define RTP_VER_SHIFT		6

#define RTP_STD_HEADER_SZ	12

// second byte
#define RTP_MARKER_MASK		0x80
#define RTP_PT_MASK			0x7f


RTPPacket::RTPPacket() {
	for (int i = 0; i < RTP_MAX_PAYLOAD_SZ; i++)
		data[i] = 0;
	size = 0;
}

RTPPacket::~RTPPacket() {
}

void RTPPacket::init(unsigned int ptype, unsigned int ssrc) {

	// set version = 2, clear marker, 0 cc
	data[0] = 2 << RTP_VER_SHIFT;
	// set sn
	htons(2,++sn);
	// set PT
	data[1] = (data[1] & RTP_MARKER_MASK) | (unsigned char)ptype;
	// ssrc
	htonl(8,ssrc);
	// clear size
	size = 0;
}

// Getters
int RTPPacket::getSize() {
	return size + RTP_STD_HEADER_SZ;
}

void RTPPacket::setPayloadSize(int sz) {
	size = sz;
}

uint8_t *RTPPacket::getData() {
	return data;
}

uint8_t *RTPPacket::getPayload()
{
	return data + RTP_STD_HEADER_SZ;
}

/* 
int RTPPacket::getPayloadSize()
{
	return size - RTP_STD_HEADER_SZ;
}

int RTPPacket::getVersion() {
	if (validateHeader()) {
		return ((data[0] & RTP_VER_MASK) >> RTP_VER_SHIFT);
	}
	return -1;
}

int	 RTPPacket::getCC() {

	if (validateHeader()) {
		return (data[0] & RTP_CC_MASK);
	}
	return -1;
}

bool RTPPacket::getM() {

	if (validateHeader()) {
		return (data[1] & RTP_MARKER_MASK);
	}
	return false;
}
int	 RTPPacket::getPT() {

	if (validateHeader()) {
		return (int(data[1]) & RTP_PT_MASK);
	}
	return -1;
}
int	 RTPPacket::getSN() {

	if (validateHeader()) {
		unsigned short sn = *((short *)&data[2]);
		return ntohs(sn);
	}
	return -1;
}
int	 RTPPacket::getTS() {

	if (validateHeader()) {
		unsigned int ts = *((int *)&data[4]);
		return ntohl(ts);
	}
	return -1;
}
int	 RTPPacket::getSSRC() {

	if (validateHeader()) {
		unsigned int ssrc = *((int *)&data[8]);
		return ntohl(ssrc);
	}
	return -1;
}
int  RTPPacket::getCSRC(int pos) {

	if (validateHeader(RTP_STD_HEADER_SZ + 4*(pos+1))) {
		unsigned int csrc = *((int *)&data[12 + pos * 4]);
		return ntohl(csrc);
	}
	return -1;
}
*/
void RTPPacket::htons(int position, unsigned short v) {
	unsigned char *s = (unsigned char *)&v;
	data[position++] = s[1];
	data[position++] = s[0];
}

void RTPPacket::htonl(int position, unsigned int v) {
	unsigned char *s = (unsigned char *)&v;
	data[position++] = s[3];
	data[position++] = s[2];
	data[position++] = s[1];
	data[position++] = s[0];
}

int RTPPacket::setPayload(uint8_t *p, int psize)
{
	if (!p)
		return 0;
	for (int i=0; i < psize; i++)
		data[i+ RTP_STD_HEADER_SZ] = p[i];
	
	size = psize;

	return psize;
}

void RTPPacket::setVersion(int i) {

	data[0] = (data[0] & ~RTP_VER_MASK) | (i << RTP_VER_SHIFT);
}

void RTPPacket::setCC(int i) {

	data[0] = (data[0] & ~RTP_CC_MASK) | i;
}

void RTPPacket::setM(bool marker) {

	if (marker)
		// set marker
		data[1] = (data[1] | RTP_MARKER_MASK);
	else
		// clear marker
		data[1] = (data[1] & ~RTP_MARKER_MASK);
}
void RTPPacket::setPT(unsigned int id) {

	// set PT
	data[1] = (data[1] & RTP_MARKER_MASK) | (unsigned char)id;
}
void RTPPacket::setSN(unsigned short seq_num) {

	sn = seq_num;
	htons(2,seq_num);
}
void RTPPacket::setTS(unsigned int ts) {

	htonl(4,ts);
}
void RTPPacket::setSSRC(unsigned int ssrc) {

	htonl(8,ssrc);
}
/*
void RTPPacket::setCSRC(int pos, int csrc) {

	if (validateHeader(RTP_STD_HEADER_SZ + 4*(pos+1))) {
		*((int *)&data[12 + pos * 4]) = htonl(csrc);
		return;
	}
}
*/