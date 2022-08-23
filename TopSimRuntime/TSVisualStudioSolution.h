#ifndef __TSVISUALSUTDIOSOLUTION_H__
#define __TSVISUALSUTDIOSOLUTION_H__

#include "TSSolutionMgr.h"

#include "TSIdlGram.h"

using namespace TSIDLGram;

struct TSVisualSutdioSolutionPrivate;

class TOPSIMIDL_DECL TSVisualStudioSolution : public TSSolution
{
	TS_MetaType(TSVisualStudioSolution,TSSolution);
public:
	TSVisualStudioSolution(void);
	~TSVisualStudioSolution(void);

	virtual bool GeneratorSolution();

private:
    TSString ReadLuaConfig();
	void InitSolutionConfig();

private:
	TSVisualSutdioSolutionPrivate * _p;
};

#endif


