#ifndef __TSTSLINUXSOLUTIONSOLUTION_H__
#define __TSTSLINUXSOLUTIONSOLUTION_H__

#include "TSSolutionMgr.h"

#include "TSIdlGram.h"

using namespace  TSIDLGram;

struct TSLinuxSolutionPrivate;

class TOPSIMIDL_DECL TSLinuxSolution : public TSSolution
{
	TS_MetaType(TSLinuxSolution,TSSolution);
public:
	TSLinuxSolution(void);
	~TSLinuxSolution(void);

	virtual bool GeneratorSolution();

private:
    TSString ReadLuaConfig();
	bool IsLinkTopicProject(const TSString & PrivateFilePath, const TSString & ProjName);
	void InitSolutionConfig();
private:
	TSLinuxSolutionPrivate * _p;
};

#endif


