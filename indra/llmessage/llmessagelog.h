// <edit>
#ifndef LL_LLMESSAGELOG_H
#define LL_LLMESSAGELOG_H
#include "stdtypes.h"
#include "llhost.h"
#include <queue>
#include <string.h>
#include "llmessagetemplate.h"
#include "lltemplatemessagereader.h"
#include "lltemplatemessagebuilder.h"
#include <boost/tokenizer.hpp>

class LLMessageSystem;
class LLMessageLogEntry
{
public:
	enum EType
	{
		TEMPLATE,
		HTTP_REQUEST,
		HTTP_RESPONSE
	};
	LLMessageLogEntry(EType type, LLHost from_host, LLHost to_host, U8* data, S32 data_size);
	LLMessageLogEntry(EType type, LLHost from_host, LLHost to_host, std::vector<U8> data, S32 data_size);
	~LLMessageLogEntry();
	EType mType;
	LLHost mFromHost;
	LLHost mToHost;
	S32 mDataSize;
	std::vector<U8> mData;
};
class LLMessageLog
{
public:
	static void setMaxSize(U32 size);
	static void setCallback(void (*callback)(LLMessageLogEntry));
	static void log(LLHost from_host, LLHost to_host, U8* data, S32 data_size);
	static std::deque<LLMessageLogEntry> getDeque();
private:
	static U32 sMaxSize;
	static void (*sCallback)(LLMessageLogEntry);
	static std::deque<LLMessageLogEntry> sDeque;
};


class LLPrettyDecodedMessage : public LLMessageLogEntry
{
public:
	LLPrettyDecodedMessage(LLMessageLogEntry entry);
	~LLPrettyDecodedMessage();
	LLUUID mID;
	U32 mSequenceID;
	std::string mName;
	std::string mSummary;
	U32 mFlags;
	std::string getFull(BOOL show_header = TRUE);
	BOOL isOutgoing();
private:
	static LLTemplateMessageReader* sTemplateMessageReader;
	static std::string getString(LLTemplateMessageReader* readerp, const char* block_name, S32 block_num, const char* var_name, e_message_variable_type var_type, BOOL &returned_hex, BOOL summary_mode = FALSE);
};

#endif
// </edit>
