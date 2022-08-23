
#include "TopSimCommTypes.h"
#include "TSAllocate.h"
#include "TSMutex.h"
#include "TSMSCSpinLock.h"
#include "TSRecursiveMutex.h"
#include "TSRWMutex.h"
#include "TSMetaType.h"
#include "TSBinaryArchive.h"
#include "TSBlackboard.h"

extern "C" TOPSIM_RT_DECL void InitializeRuntime()
{
	//设置日志及域对象资源限制
	pugi::xpath_node_set nodes = GetCfgDoc().select_nodes("Config/ExtendConfigs/ExtendConfig");
	for (pugi::xpath_node_set::const_iterator it = nodes.begin(); it != nodes.end(); ++it)
	{
		TSString Key = it->node().attribute("Key").as_string();
		
		if (!Key.compare("LINK_LOG_CONSOLE") && !LINK_LOG_CONSOLE)
		{
			UINT32 Value = it->node().attribute("Value").as_uint();
			LINK_LOG_CONSOLE = Value;
		}
		else if (!Key.compare("LINK_LOG_VERBOSE") && !LINK_LOG_VERBOSE)
		{
			UINT32 Value = it->node().attribute("Value").as_uint();
			LINK_LOG_VERBOSE = Value;
		}
		else if (!Key.compare("LINK_LOG_MIDDLEWARE_VERBOSE") && !LINK_LOG_MIDDLEWARE_VERBOSE)
		{
			UINT32 Value = it->node().attribute("Value").as_uint();
			LINK_LOG_MIDDLEWARE_VERBOSE = Value;
		}
		else if (!Key.compare("LINK_DOMAIN_OBJECT_LIMIT") && 1024 == LINK_DOMAIN_OBJECT_LIMIT)
		{
			UINT32 Value = it->node().attribute("Value").as_uint();
			LINK_DOMAIN_OBJECT_LIMIT = Value;
		}
	}

	TSString DirectIsUpdate_ = TSTopicTypeManager::Instance()->GetExtendCfg("DirectIsUpdate");
	if()

	//添加默认域和默认主题TOPIC_CMD的发布订阅者
	T_D();

#ifndef WITHOUT_LICENSE

	DEF_LOG_KERNEL("checkin middleware license system...");

#if defined(XSIM3_2) || defined(XSIM3_3)

	INIT_LICSENCE_JOB();
	if (!TRY_USE_LICSENCE(FEATURE_XENGINE))
	{
		DEF_LOG_ERROR("license system initialize failed!");
		RELEASE_LICSENCE(FEATURE_XENGINE);
		return false;
	}

#else

	int Ret = 0;
	//检测当前工作目录下是否有TopSimInt.dat文件
	if (TSDir::Exists("TopSimInt.dat"))
	{
		Ret = TSLicenseSystemInitialize();
	}
	else
	{
		char * linkPath = getenv("LINKSDK");
		Ret = TSLicenseSystemInitialize((TSString(linkPath) + "/TopSimInt.dat").c_str(), linkPath);
#ifdef __android_AR__	//安卓AR眼镜c#专用
		if (Ret != 0)
		{
			TSString linkpath = TSTopicTypeManager::Instance()->GetExtendCfg("LINKSDK");
			Ret = TSLicenseSystemInitialize((linkpath + "/TopSimInt.dat").c_str(), linkpath.c_str());
		}
#endif
	}

	if (Ret != 0)
	{
		TSLicenseSystemCleanup();
		DEF_LOG_FATAL("license system initialize failed! error code :%1.") << Ret;

		return Ret;
	}

	Ret = TSLicenseCheckin(LICENSE_PRODUCT_ID);

	if (Ret < 0)
	{
		DEF_LOG_FATAL("license system chechin failed! product id %1,error code :%2.") << LICENSE_PRODUCT_ID << Ret;

		return false;
	}
#endif

#else

	DEF_LOG_KERNEL("checkin ua license system...");

#endif

	BUILDIN_DOMAIN_ID = "232";
	Mask = DDS_STATUS_MASK_ALL;

	if (!Mask || BUILDIN_DOMAIN_ID.empty())
	{
#if defined(XSIM3_2) || defined(XSIM3_3)
		DEF_LOG_ERROR("license login failed! feature id:%1.") << LICENSE_FEATURE_MIDDLEWARE_MASK;
		RELEASE_LICSENCE(FEATURE_XENGINE);
#else
		DEF_LOG_FATAL("license login failed! feature id:%1.") << LICENSE_FEATURE_MIDDLEWARE_MASK;
#endif

		return false;
	}

	if(!d->_Inited)
	{
		d->_Inited = true;
		d->_IsUseSelfParticipantQosPolicy = false;
#ifdef _MSC_VER
#if	 _MSC_VER >= 1900

#if !defined(XSIM3_2) && !defined(XSIM3_3)
		if (TSDir::Exists(QOSCONFIG))
		{
			d->_Operator->InitializeLua(QOSCONFIG);
		}
#endif

#endif
#endif
		NDDSConfigLogger::get_instance()->set_verbosity_by_category(NDDS_CONFIG_LOG_CATEGORY_ALL,
            (NDDS_Config_LogVerbosity)LINK_LOG_MIDDLEWARE_VERBOSE);

		//big object interface

		pugi::xpath_node_set nodes = TSTopicTypeManager::Instance()->GetConfigureDoc().select_nodes("Config/BigObjectInterfaces/Interface");
		for (pugi::xpath_node_set::const_iterator it = nodes.begin(); it != nodes.end(); ++it)
		{
			TSString InterfaceName = it->node().attribute("Name").as_string();

			DDS_Duration_t FastHeartbeatPeriod = DDS_Duration_t::from_millis(it->node().attribute("FastHeartbeatPeriod").as_uint(100));

			if (!InterfaceName.empty())
			{
				d->_BigObjectInterfaces.insert(std::make_pair(InterfaceName, FastHeartbeatPeriod));
			}
		}

		//content filter topic

		nodes = TSTopicTypeManager::Instance()->GetConfigureDoc().select_nodes("Config/ContentFilterTopics/ContentFilterTopic");
		for (pugi::xpath_node_set::const_iterator it = nodes.begin(); it != nodes.end(); ++it)
		{
			TSString TopicName = it->node().attribute("Name").as_string();

			if (!TopicName.empty())
			{
				d->_ContentFilterTopics.insert(TopicName);
			}
		}
		
		//Outers
		nodes = TSTopicTypeManager::Instance()->GetConfigureDoc().select_nodes("Config/Outers/Outer");
		for (pugi::xpath_node_set::const_iterator it = nodes.begin(); it != nodes.end(); ++it)
		{
			TSString TopicName = it->node().attribute("Name").as_string();

			if (!TopicName.empty())
			{
				d->_Outers.insert(TopicName);
			}
		}

		//Inters
		nodes = TSTopicTypeManager::Instance()->GetConfigureDoc().select_nodes("Config/Inters/Inter");
		for (pugi::xpath_node_set::const_iterator it = nodes.begin(); it != nodes.end(); ++it)
		{
			TSString TopicName = it->node().attribute("Name").as_string();

			if (!TopicName.empty())
			{
				d->_Inters.insert(TopicName);
			}
		}

		//ReliableConfigure
		nodes = TSTopicTypeManager::Instance()->GetConfigureDoc().select_nodes("Config/ReliableConfigures/ReliableConfigure");
		for (pugi::xpath_node_set::const_iterator it = nodes.begin(); it != nodes.end(); ++it)
		{
			TSString TopicName = it->node().attribute("Name").as_string();
			if (!TopicName.empty())
			{
				ReliableConfigure Configure;

				TSString ReliableTypeStr = it->node().attribute("ReliableType").as_string();

				if (ReliableTypeStr == "Strict")
				{
					Configure.ReliabilityKind = DDS_RELIABLE_RELIABILITY_QOS;
					Configure.HistoryKind = DDS_KEEP_ALL_HISTORY_QOS;
				}
				else if (ReliableTypeStr == "BestEffort")
				{
					Configure.ReliabilityKind = DDS_BEST_EFFORT_RELIABILITY_QOS;
					Configure.HistoryKind = DDS_KEEP_LAST_HISTORY_QOS;
				}
				else if (ReliableTypeStr == "KeepLast")
				{
					Configure.ReliabilityKind = DDS_RELIABLE_RELIABILITY_QOS;
					Configure.HistoryKind = DDS_KEEP_LAST_HISTORY_QOS;
				}

				Configure.Depth = it->node().attribute("Depth").as_uint(1);

				bool bDurable = it->node().attribute("Durable").as_bool();

				if (bDurable)
				{
					Configure.DurabilityKind = DDS_TRANSIENT_LOCAL_DURABILITY_QOS;
					Configure.ReliabilityKind = DDS_RELIABLE_RELIABILITY_QOS;
				}
				
				TSString PublishModeStr = it->node().attribute("PublishMode").as_string();

				if (PublishModeStr == "Sync")
				{
					Configure.PublishModeKind = DDS_SYNCHRONOUS_PUBLISH_MODE_QOS;
				}
				else if (PublishModeStr == "AsyncReliable")
				{
					Configure.ReliabilityKind = DDS_RELIABLE_RELIABILITY_QOS;
					Configure.PublishModeKind = DDS_ASYNCHRONOUS_PUBLISH_MODE_QOS;
				}
				else if (PublishModeStr == "AsyncReliableAll")
				{
					Configure.ReliabilityKind = DDS_RELIABLE_RELIABILITY_QOS;
					Configure.PublishModeKind = DDS_ASYNCHRONOUS_PUBLISH_MODE_QOS;
					Configure.HistoryKind = DDS_KEEP_ALL_HISTORY_QOS;
				}

				Configure.FastHeartbeatPeriod = DDS_Duration_t::from_millis(it->node().attribute("FastHeartbeatPeriod").as_uint(3000));

				d->_ReliableConfigures.insert(std::make_pair(TopicName, Configure));
			}
		}



		if(!LINK_LOG_CONSOLE)
		{

			TSString TimeString = TIME2CSTR(boost::posix_time::second_clock::local_time());
#ifndef __VXWORKS__
			boost::algorithm::replace_all(TimeString,TS_TEXT(" "),TS_TEXT("_"));
			boost::algorithm::replace_all(TimeString,TS_TEXT(":"),TS_TEXT("_"));
			boost::algorithm::replace_all(TimeString,TS_TEXT("-"),TS_TEXT("_"));
#else
			for(int i = 0 ; i < TimeString.size() ; ++i)
			{
				if((TimeString.at(i) == TS_TEXT(' ')) || (TimeString.at(i) == TS_TEXT(':')) || (TimeString.at(i) == TS_TEXT('-')))
				{
					TimeString.at(i) = TS_TEXT('_');
				}
			}
#endif
			//绝对路径在这不好使，必须手动自己创建perf文件夹才行
#ifndef __VXWORKS__
			TSString FilePath = (boost::filesystem::current_path()/TS_TEXT("log/perf/")).string();
			if(!boost::filesystem::exists(FilePath))
			{

				boost::filesystem::create_directories(FilePath);

			}
#else
			TSString FilePath = TSDir::CurrentPath() + TS_TEXT("/log/perf/");
			if(!TSDir::Exists(FilePath))
			{
				TSDir::CreateDirectory(FilePath);
			}
#endif

			d->_FileBaseName = FilePath + TimeString + "_" + TSValue_Cast<TSString>(GetCRC32(TSOSMisc::GenerateUUID())) ;

			TSString FileFullName = d->_FileBaseName + ".middleware";
			if(d->_logOutputFile = fopen(FileFullName.c_str(),"w+"))
			{
				NDDSConfigLogger::get_instance()->set_output_file(d->_logOutputFile);
			}
		}

		DDS_DomainParticipantFactoryQos facQos;
		DDSTheParticipantFactory->get_qos(facQos);
		facQos.resource_limits.max_objects_per_thread = LINK_DOMAIN_OBJECT_LIMIT;
		DDSTheParticipantFactory->set_qos(facQos);

		if(LINK_ENABLED_TOPIC_REMOVE)
        {
            if (CreateDomain(BUILDIN_DOMAIN_ID, TSFrontAppUniquelyId()))
            {
                if (!Publish(BUILDIN_DOMAIN_ID, __topic_internal_remove__))
                {
                    DEF_LOG_ERROR(TRS_TOPSIMMIDDLEWARE_TSTRANSFERMIDDLEWARE_CPP_2689);

                    d->_Inited = false;

#if defined(XSIM3_2) || defined(XSIM3_3)
					RELEASE_LICSENCE(FEATURE_XENGINE);
#endif
                    return false;
                }

                if (!Subscribe(BUILDIN_DOMAIN_ID, __topic_internal_remove__))
                {
#if defined(XSIM3_2) || defined(XSIM3_3)
					DEF_LOG_ERROR(TRS_TOPSIMMIDDLEWARE_TSTRANSFERMIDDLEWARE_CPP_2697);
					RELEASE_LICSENCE(FEATURE_XENGINE);
#else
					DEF_LOG_FATAL(TRS_TOPSIMMIDDLEWARE_TSTRANSFERMIDDLEWARE_CPP_2697);
#endif
                    d->_Inited = false;

                    return false;
                }

#if defined(XSIM3_2) || defined(XSIM3_3)
				RELEASE_LICSENCE(FEATURE_XENGINE);
#endif
                return true;
            }
            else
            {
                d->_Inited = false;

#if defined(XSIM3_2) || defined(XSIM3_3)
				RELEASE_LICSENCE(FEATURE_XENGINE);
#endif

                return false;
            }
        }

#if defined(XSIM3_2) || defined(XSIM3_3)
		RELEASE_LICSENCE(FEATURE_XENGINE);
#endif

        return true;

	}

	d->_Inited = false;

#if defined(XSIM3_2) || defined(XSIM3_3)
	//RELEASE_LICSENCE(FEATURE_XENGINE);
#endif

	return false;
}