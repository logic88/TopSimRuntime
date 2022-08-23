#include "stdafx.h"

#include <boost/foreach.hpp>
#include <tbb/concurrent_queue.h>

#include "TSGramCommon.h"


namespace TSGramCommon
{
    void ExceptionTextParser::OutputLocation( OutputPipe * Pipe, UINT32 nLine)
    {
        SetPos(mCurIter);

        char line[256];
        Iterator pFirst = GetPos();
        while (pFirst > mBegin && *pFirst != '\n')
            pFirst--;
        if (*pFirst == '\n')
            ++pFirst;
        Iterator pLast = GetPos();
        while (pLast < mEnd && *pLast != '\n')
            pLast++;
        size_t n = pLast - pFirst;
        n = n < 254 ? n : 254;
        strncpy(line, pFirst, n);
        line[n] = '\0';

        char marker[256];
        n = GetPos() - pFirst;
        n = n < 254 ? n : 254;
        for (size_t i=0; i < n; ++i)
            marker[i] = ' ';
        marker[n] = '^';
        marker[n + 1] = '\0';

        // Count lines
        int nline = 1;
        for (Iterator i = mBegin; i < pFirst; ++i) {
            if (*i == '\n') {
                ++nline;
            }
        }

		mBeginCount = nLine;

        TSString OutputMsg;

        if(mFileName.empty())
        {
            OutputMsg = 
				TSStringUtil::Format(" (%d).%s\n",nline,line);
           
        }
        else
        {
            OutputMsg = mFileName + TSStringUtil::Format(" Line(%d)", nline+mBeginCount) + GB18030ToTSString(":´íÎó:") + line;
				//TSStringUtil::Format("%s (%d): ´íÎó: %s\n",mFileName.c_str(),nline,line);
        }

        if(Pipe)
        {
            Pipe->Output(OutputMsg.c_str());
        }
        else
        {
            STD_COUT << OutputMsg;
        }

		if (OutputMsg.find(GB18030ToTSString("´íÎó")) != TSString::npos)
		{
			throw TSException(OutputMsg);
		}
    }

    UINT32 ExceptionTextParser::GetLineCount( Iterator Pos )
    {
        Iterator pFirst = Pos;
        while (pFirst > mBegin && *pFirst != '\n')
            pFirst--;
        if (*pFirst == '\n')
            ++pFirst;
        int nline = 1;
        for (Iterator i = mBegin; i < pFirst; ++i) {
            if (*i == '\n') {
                ++nline;
            }
        }

        return nline + mBeginCount;
    }

    TSString ExceptionTextParser::GetlineMsg( UINT32 Line )
    {
        Iterator pFirst = mBegin;
        for (Iterator i = mBegin; i < mEnd; ++i) {
            if(Line <= 1)
            {
                pFirst = i;
                break;
            }
            else if (*i == '\n') {
                --Line;
            }
        }

        Iterator pLast = pFirst;

        while (pLast < mEnd && *pLast != '\n')
            pLast++;

        return TSString(pFirst,pLast);
    }

    UINT32 ExceptionTextParser::GetMaxLine()
    {
        return GetLineCount(End());
    }

    const TSString & ExceptionTextParser::GetFullFileName()
    {
        return mFileName;
    }

    void VisitChild( ExceptionTextParser::Node * AstNode,VisitProc Proc )
    {
        if(ExceptionTextParser::Node * child = AstNode->GetFirstChild())
        {
            Visit(child,Proc);
        }
    }

    void Visit(ExceptionTextParser::Node * AstNode,VisitProc Proc)
    {
       
        if(AstNode->IsCompleted())
        {
            Proc(AstNode);
        }

        VisitChild(AstNode,Proc);

        if(ExceptionTextParser::Node * NextSibling = AstNode->GetSibling())
        {
            Visit(NextSibling,Proc);
        }
    }
}



