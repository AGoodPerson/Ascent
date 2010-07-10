//<edit>
//2010 Hazim Gazov
//WTFPL v2

#include "llviewerprecompiledheaders.h"
#include "llfloatermessagetamperer.h"
#include "lluictrlfactory.h"
#include "llscrolllistctrl.h"
#include "llcombobox.h"
#include "llcheckboxctrl.h"
#include "lltexteditor.h"
#include "llviewerwindow.h" // alertXml
#include "llmessagetemplate.h"
#include "llmessageconfig.h"
#include "llmessagetamperer.h"
#include <boost/tokenizer.hpp>
#include "llmenugl.h"
#include "llmessagetamperinghelper.h"

//todo: separate tamperer from floater

LLFloaterMessageTamperer* LLFloaterMessageTamperer::sInstance;

LLFloaterMessageTamperer::LLFloaterMessageTamperer()
:	LLFloater()
{
	sInstance = this;
	LLMessageTamperingHelper::init();
	LLUICtrlFactory::getInstance()->buildFloater(this, "floater_message_tamperer.xml");
}
LLFloaterMessageTamperer::~LLFloaterMessageTamperer()
{
	sInstance = NULL;
}
// static
void LLFloaterMessageTamperer::show()
{
	if(!sInstance) sInstance = new LLFloaterMessageTamperer();
	sInstance->open();
}
BOOL LLFloaterMessageTamperer::postBuild()
{

	//populate the map of messages to tamper
	if(LLMessageTamperer::tamperedTypes.empty())
	{
		std::vector<std::string> names;
		LLMessageSystem::message_template_name_map_t::iterator temp_end = gMessageSystem->mMessageTemplates.end();
		LLMessageSystem::message_template_name_map_t::iterator temp_iter;
		std::vector<std::string>::iterator names_end;
		std::vector<std::string>::iterator names_iter;
		for(temp_iter = gMessageSystem->mMessageTemplates.begin(); temp_iter != temp_end; ++temp_iter)
			names.push_back((*temp_iter).second->mName);
		std::sort(names.begin(), names.end());

		names_end = names.end();

		for(names_iter = names.begin(); names_iter != names_end; ++names_iter)
		{
			LLMessageTamperer::tamperedTypes[(*names_iter)] = MessageDirection::NONE;
		}

		names.clear();
	}

	refreshTamperedMessages();

	return TRUE;
}

void LLFloaterMessageTamperer::refreshTamperedMessages()
{
	LLScrollListCtrl* scrollp = getChild<LLScrollListCtrl>("message_type_list");
	
	// Rebuild scroll list from scratch
	LLUUID selected_id = scrollp->getFirstSelected() ? scrollp->getFirstSelected()->getUUID() : LLUUID::null;
	S32 scroll_pos = scrollp->getScrollPos();
	scrollp->clearRows();
	
	std::map<std::string, int>::iterator message_types_end = LLMessageTamperer::tamperedTypes.end();
	std::map<std::string, int>::iterator message_types_iter;

	for(message_types_iter = LLMessageTamperer::tamperedTypes.begin(); message_types_iter != message_types_end; ++message_types_iter)
	{
		LLSD element;

		int direction = (*message_types_iter).second;

		LLSD& rule_column = element["columns"][0];
		rule_column["column"] = "MessageType";
		rule_column["value"] = (*message_types_iter).first;

		LLSD& tamper_in_column = element["columns"][1];
		tamper_in_column["column"] = "tamper_in";
		tamper_in_column["type"] = "checkbox";

		LLSD& tamper_out_column = element["columns"][2];
		tamper_out_column["column"] = "tamper_out";
		tamper_out_column["type"] = "checkbox";

		LLScrollListItem* scroll_itemp = scrollp->addElement(element);
		LLScrollListCheck* tamper_in_elem = (LLScrollListCheck*)scroll_itemp->getColumn(1);
		LLScrollListCheck* tamper_out_elem = (LLScrollListCheck*)scroll_itemp->getColumn(2);

		if((direction & MessageDirection::INBOUND) && (direction & MessageDirection::OUTBOUND))
		{
			tamper_in_elem->setValue(TRUE);
			tamper_out_elem->setValue(TRUE);
		}
		else if(direction & MessageDirection::INBOUND)
		{
			tamper_in_elem->setValue(TRUE);
			tamper_out_elem->setValue(FALSE);
		}
		else if(direction & MessageDirection::OUTBOUND)
		{
			tamper_in_elem->setValue(FALSE);
			tamper_out_elem->setValue(TRUE);
		}
		else
		{
			tamper_in_elem->setValue(FALSE);
			tamper_out_elem->setValue(FALSE);
		}

		//holy crap, this almost looks like lisp
		tamper_in_elem->setClickCallback(onClickTamperIn, (void*)((*message_types_iter).first.c_str()));
		tamper_out_elem->setClickCallback(onClickTamperOut, (void*)((*message_types_iter).first.c_str()));
	}
	
	if(selected_id.notNull()) scrollp->selectByID(selected_id);
	if(scroll_pos < scrollp->getItemCount()) scrollp->setScrollPos(scroll_pos);
}

//static
BOOL LLFloaterMessageTamperer::onClickTamperIn(void* user_data)
{
	std::string messageType = (const char*)user_data;

	if(LLMessageTamperer::tamperedTypes[messageType] & MessageDirection::INBOUND)
	{
		LLMessageTamperer::tamperedTypes[messageType] = LLMessageTamperer::tamperedTypes[messageType] & MessageDirection::INBOUND;
		LLMessageTamperer::isAnythingTampered();
	}
	else
	{
		LLMessageTamperer::tamperedTypes[messageType] = LLMessageTamperer::tamperedTypes[messageType] | MessageDirection::INBOUND;
		LLMessageTamperer::tamperingAny = true;
	}

	return true;
}

//static
BOOL LLFloaterMessageTamperer::onClickTamperOut(void* user_data)
{
	std::string messageType = (const char*)user_data;

	if(LLMessageTamperer::tamperedTypes[messageType] & MessageDirection::OUTBOUND)
	{
		LLMessageTamperer::tamperedTypes[messageType] = LLMessageTamperer::tamperedTypes[messageType] & MessageDirection::OUTBOUND;
		LLMessageTamperer::isAnythingTampered();
	}
	else
	{
		LLMessageTamperer::tamperedTypes[messageType] = LLMessageTamperer::tamperedTypes[messageType] | MessageDirection::OUTBOUND;
		LLMessageTamperer::tamperingAny = true;
	}

	return true;
}

// </edit>
