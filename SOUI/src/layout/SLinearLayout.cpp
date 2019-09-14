﻿#include "souistd.h"
#include "layout\SLinearLayout.h"
#include "helper\SplitString.h"

namespace SOUI
{

    SLinearLayoutParam::SLinearLayoutParam()
	{
		Clear();
	}

    bool SLinearLayoutParam::IsMatchParent(ORIENTATION orientation) const
    {
        switch(orientation)
        {
        case Horz:
            return width.isMatchParent();
        case Vert:
            return height.isMatchParent();
        case Any:
            return IsMatchParent(Horz)|| IsMatchParent(Vert);
        case Both:
        default:
            return IsMatchParent(Horz) && IsMatchParent(Vert);
        }
    }

	bool SLinearLayoutParam::IsWrapContent(ORIENTATION orientation) const
	{
        switch(orientation)
        {
        case Horz:
            return width.isWrapContent();
        case Vert:
            return height.isWrapContent();
        case Any:
            return IsWrapContent(Horz)|| IsWrapContent(Vert);
        case Both:
        default:
            return IsWrapContent(Horz) && IsWrapContent(Vert);
        }
	}

    bool SLinearLayoutParam::IsSpecifiedSize(ORIENTATION orientation) const
    {
        switch(orientation)
        {
        case Horz:
            return width.isSpecifiedSize();
        case Vert:
            return height.isSpecifiedSize();
        case Any:
            return IsSpecifiedSize(Horz)|| IsSpecifiedSize(Vert);
        case Both:
        default:
            return IsSpecifiedSize(Horz) && IsSpecifiedSize(Vert);
        }

    }

	SLayoutSize SLinearLayoutParam::GetSpecifiedSize(ORIENTATION orientation) const
    {
        switch(orientation)
        {
        case Horz:
            return width;
        case Vert:
            return height;
        case Any: 
        case Both:
        default:
            SASSERT_FMTA(FALSE,"GetSpecifiedSize can only be applied for Horz or Vert");
            return SLayoutSize();
        }
    }


    HRESULT SLinearLayoutParam::OnAttrSize(const SStringW & strValue,BOOL bLoading)
    {
		SStringWList szStr ;
		if(2!=SplitString(strValue,L',',szStr)) return E_FAIL;

		OnAttrWidth(szStr[0],bLoading);
		OnAttrHeight(szStr[1],bLoading);
        return S_OK;
    }

    HRESULT SLinearLayoutParam::OnAttrWidth(const SStringW & strValue,BOOL bLoading)
    {
        if(strValue.CompareNoCase(L"matchParent") == 0)
			width.setMatchParent();
        else if(strValue.CompareNoCase(L"wrapContent") == 0)
			width.setWrapContent();
        else
			width = GETLAYOUTSIZE(strValue);
        return S_OK;
    }

    HRESULT SLinearLayoutParam::OnAttrHeight(const SStringW & strValue,BOOL bLoading)
    {
        if(strValue.CompareNoCase(L"matchParent") == 0)
            height.setMatchParent();
        else if(strValue.CompareNoCase(L"wrapContent") == 0)
            height.setWrapContent();
        else
			height = GETLAYOUTSIZE(strValue);
        return S_OK;
    }


	HRESULT SLinearLayoutParam::OnAttrExtend(const SStringW & strValue,BOOL bLoading)
	{
		SStringWList strList;
		size_t nSeg = SplitString(strValue,L',',strList);
		if(nSeg==1)
		{
			extend_left = 
				extend_top =
				extend_right =
				extend_bottom = GETLAYOUTSIZE(strList[0]);
			return S_OK;
		}else if(nSeg == 4)
		{
			extend_left = GETLAYOUTSIZE(strList[0]);
			extend_top = GETLAYOUTSIZE(strList[1]);
			extend_right = GETLAYOUTSIZE(strList[2]);
			extend_bottom = GETLAYOUTSIZE(strList[3]);
			return S_OK;
		}
		return E_INVALIDARG;
	}

	void SLinearLayoutParam::Clear()
	{
		width.setWrapContent();
		height.setWrapContent();
		weight = 0.0f;
		gravity = G_Undefined;
	}

	void SLinearLayoutParam::SetMatchParent(ORIENTATION orientation)
	{
        switch(orientation)
        {
        case Horz:
            width.setMatchParent();
            break;
        case Vert:
            height.setMatchParent();
            break;
        case Both:
            width.setMatchParent();
			height.setMatchParent();
            break;
        }
	}

	void SLinearLayoutParam::SetWrapContent(ORIENTATION orientation)
	{
        switch(orientation)
        {
        case Horz:
            width.setWrapContent();
            break;
        case Vert:
            height.setWrapContent();
            break;
        case Both:
            width.setWrapContent();
			height.setWrapContent();
            break;
        }
	}

	void SLinearLayoutParam::SetSpecifiedSize(ORIENTATION orientation, const SLayoutSize& layoutSize)
	{
        switch(orientation)
        {
        case Horz:
            width = layoutSize;
            break;
        case Vert:
            height = layoutSize;
            break;
        case Both:
            width = height = layoutSize;
            break;
        }

	}

	void * SLinearLayoutParam::GetRawData()
	{
		return (SLinearLayoutParamStruct*)this;
	}

	ILayoutParam * SLinearLayoutParam::Clone() const
	{
		SLinearLayoutParam *pRet = new SLinearLayoutParam();
		memcpy(pRet->GetRawData(), (SLinearLayoutParamStruct*)this, sizeof(SLinearLayoutParamStruct));
		return pRet;
	}

	//////////////////////////////////////////////////////////////////////////
    SLinearLayout::SLinearLayout(void):m_gravity(G_Undefined)
    {
    }

    SLinearLayout::~SLinearLayout(void)
    {
    }

    void SLinearLayout::LayoutChildren(SWindow * pParent)
    {
        CRect rcParent = pParent->GetChildrenLayoutRect();
		        
        SIZE *pSize = new SIZE [pParent->GetChildrenCount()];
		SWindow ** pChilds = new SWindow * [pParent->GetChildrenCount()];
		memset(pSize,0,sizeof(SIZE)*pParent->GetChildrenCount());

		int nChilds = 0;
		
        int offset = 0;
        float fWeight= 0.0f;
		int interval = m_interval.toPixelSize(pParent->GetScale());

        {//assign width or height

            int iChild = 0;

			SWindow *pChild=pParent->GetNextLayoutChild(NULL);
            while(pChild)
            {
				SLinearLayoutParam *pLinearLayoutParam = pChild->GetLayoutParamT<SLinearLayoutParam>();

				int nScale = pChild->GetScale();

                CSize szChild(SIZE_WRAP_CONTENT,SIZE_WRAP_CONTENT);
                if(pLinearLayoutParam->IsMatchParent(Horz))
                    szChild.cx = rcParent.Width();
                else if(pLinearLayoutParam->IsSpecifiedSize(Horz))
                {
                    szChild.cx = pLinearLayoutParam->GetSpecifiedSize(Horz).toPixelSize(nScale);
                    szChild.cx += pLinearLayoutParam->extend_left.toPixelSize(nScale) + pLinearLayoutParam->extend_right.toPixelSize(nScale);
                }

                if(pLinearLayoutParam->IsMatchParent(Vert))
                    szChild.cy = rcParent.Height();
                else if(pLinearLayoutParam->IsSpecifiedSize(Vert))
                {
                    szChild.cy = pLinearLayoutParam->GetSpecifiedSize(Vert).toPixelSize(nScale);
                    szChild.cy += pLinearLayoutParam->extend_top.toPixelSize(nScale) + pLinearLayoutParam->extend_bottom.toPixelSize(nScale);
                }
                
                if(pLinearLayoutParam->weight > 0.0f)
                {
                    fWeight += pLinearLayoutParam->weight;
                }

                if(szChild.cx == SIZE_WRAP_CONTENT || szChild.cy == SIZE_WRAP_CONTENT)
                {
                    CSize szCalc = pChild->GetDesiredSize(szChild.cx,szChild.cy);
                    if(szChild.cx == SIZE_WRAP_CONTENT)
                    {
                        szChild.cx = szCalc.cx;
                        szChild.cx += pLinearLayoutParam->extend_left.toPixelSize(nScale) + pLinearLayoutParam->extend_right.toPixelSize(nScale);
                    }
                    if(szChild.cy == SIZE_WRAP_CONTENT) 
                    {
                        szChild.cy = szCalc.cy;
                        szChild.cy += pLinearLayoutParam->extend_top.toPixelSize(nScale) + pLinearLayoutParam->extend_bottom.toPixelSize(nScale);
                    }
                }

				pChilds[iChild] = pChild;
                pSize [iChild] = szChild;
                offset += m_orientation == Vert ? szChild.cy:szChild.cx;

				offset += interval;//add interval

				iChild++;
				pChild=pParent->GetNextLayoutChild(pChild);
            }

			nChilds = iChild;
			offset -= interval;//sub the last interval value.
        }


        int size = m_orientation == Vert? rcParent.Height():rcParent.Width();
		ORIENTATION orienOther = m_orientation == Vert?Horz:Vert;
        if(fWeight > 0.0f && size > offset)
        {//assign size by weight
            int nRemain = size - offset;						

			for(int iChild = 0;iChild < nChilds;iChild ++)
            {
				if (SLayoutSize::fequal(fWeight, 0.0f))
					break;
				SWindow *pChild = pChilds[iChild];
				SLinearLayoutParam *pLinearLayoutParam = pChild->GetLayoutParamT<SLinearLayoutParam>();
				int nScale = pChild->GetScale();
                if(pLinearLayoutParam->weight > 0.0f)
                {
					int extra = int(nRemain*pLinearLayoutParam->weight / fWeight + 0.5f);
					LONG & szChild = m_orientation == Vert? pSize[iChild].cy:pSize[iChild].cx;
                    szChild += extra;
					nRemain -= extra;
					fWeight -= pLinearLayoutParam->weight;

					if(pLinearLayoutParam->IsWrapContent(orienOther))
					{
						ILayoutParam * backup = pLinearLayoutParam->Clone();
						pLinearLayoutParam->SetSpecifiedSize(m_orientation,SLayoutSize((float)szChild,SLayoutSize::dp));
						int nWid = pSize[iChild].cx, nHei = pSize[iChild].cy;

						if(orienOther == Vert) 
							nHei = SIZE_WRAP_CONTENT;
						else 
							nWid = SIZE_WRAP_CONTENT;

						CSize szCalc = pChild->GetDesiredSize(nWid,nHei);
						if(orienOther == Vert)
						{
							szCalc.cy += pLinearLayoutParam->extend_top.toPixelSize(nScale) + pLinearLayoutParam->extend_bottom.toPixelSize(nScale);
							pSize[iChild].cy = szCalc.cy;
						}
						else
						{
							szCalc.cx += pLinearLayoutParam->extend_left.toPixelSize(nScale) + pLinearLayoutParam->extend_right.toPixelSize(nScale);
							pSize[iChild].cx = szCalc.cx;
						}
						pChild->SetLayoutParam(backup);
						backup->Release();
					}
                }
            }
        }
        {//assign position
            offset = 0;
			for(int iChild = 0;iChild < nChilds;iChild ++)
			{
				SWindow *pChild = pChilds[iChild];

                SLinearLayoutParam *pLinearLayoutParam = pChild->GetLayoutParamT<SLinearLayoutParam>();
				int nScale = pChild->GetScale();
                Gravity gravity = pLinearLayoutParam->gravity == G_Undefined? m_gravity:pLinearLayoutParam->gravity;
                if(gravity == G_Undefined) gravity = G_Left;

                if(m_orientation == Vert)
                {
                    CRect rcChild(CPoint(0,offset),pSize[iChild]);
                    rcChild.OffsetRect(rcParent.TopLeft());
                    if(gravity == G_Center)
                        rcChild.OffsetRect((rcParent.Width()-rcChild.Width())/2,0);
                    else if(gravity == G_Right)
                        rcChild.OffsetRect(rcParent.Width()-rcChild.Width(),0);
                    
                    CRect rcChild2 = rcChild;
                    rcChild2.DeflateRect(pLinearLayoutParam->extend_left.toPixelSize(nScale),
						pLinearLayoutParam->extend_top.toPixelSize(nScale),
						pLinearLayoutParam->extend_right.toPixelSize(nScale),
						pLinearLayoutParam->extend_bottom.toPixelSize(nScale)
						);

                    pChild->OnRelayout(rcChild2);

                    offset += rcChild.Height();
                }else
                {
                    CRect rcChild(CPoint(offset,0),pSize[iChild]);
                    rcChild.OffsetRect(rcParent.TopLeft());
                    if(gravity == G_Center)
                        rcChild.OffsetRect(0,(rcParent.Height()-rcChild.Height())/2);
                    else if(gravity == G_Right)
                        rcChild.OffsetRect(0,rcParent.Height()-rcChild.Height());

                    CRect rcChild2 = rcChild;
					rcChild2.DeflateRect(pLinearLayoutParam->extend_left.toPixelSize(nScale),
						pLinearLayoutParam->extend_top.toPixelSize(nScale),
						pLinearLayoutParam->extend_right.toPixelSize(nScale),
						pLinearLayoutParam->extend_bottom.toPixelSize(nScale));

                    pChild->OnRelayout(rcChild2);

                    offset += rcChild.Width();
                }
				offset += interval;
            }

        }

		delete []pChilds;
		delete []pSize;

    }

	//nWidth,nHeight == -1:wrap_content
	CSize SLinearLayout::MeasureChildren(const SWindow * pParent,int nWidth,int nHeight) const
	{
		SIZE *pSize = new SIZE [pParent->GetChildrenCount()];
		memset(pSize,0,sizeof(SIZE)*pParent->GetChildrenCount());
		SWindow ** ppChilds = new SWindow *[pParent->GetChildrenCount()];

        ILayoutParam * pParentLayoutParam = pParent->GetLayoutParam();

		float fWeight = 0;
        int nChilds = 0;
        {
			int iChild = 0;

			SWindow *pChild = pParent->GetNextLayoutChild(NULL);
			while(pChild)
			{
				SLinearLayoutParam *pLinearLayoutParam = pChild->GetLayoutParamT<SLinearLayoutParam>();
				int nScale = pChild->GetScale();
				CSize szChild(SIZE_WRAP_CONTENT,SIZE_WRAP_CONTENT);
				if(pLinearLayoutParam->IsMatchParent(Horz))
				{
					if(!pParentLayoutParam->IsWrapContent(Horz))
						szChild.cx = nWidth;
				}
				else if(pLinearLayoutParam->IsSpecifiedSize(Horz))
				{
					szChild.cx = pLinearLayoutParam->GetSpecifiedSize(Horz).toPixelSize(nScale);
					szChild.cx += pLinearLayoutParam->extend_left.toPixelSize(nScale) + pLinearLayoutParam->extend_right.toPixelSize(nScale);
				}
				if(pLinearLayoutParam->IsMatchParent(Vert))
				{
					if(!pParentLayoutParam->IsWrapContent(Vert))
						szChild.cy = nHeight;
				}
				else if(pLinearLayoutParam->IsSpecifiedSize(Vert))
				{
					szChild.cy = pLinearLayoutParam->GetSpecifiedSize(Vert).toPixelSize(nScale);
					szChild.cy += pLinearLayoutParam->extend_top.toPixelSize(nScale) + pLinearLayoutParam->extend_bottom.toPixelSize(nScale);
				}
				if(szChild.cx == SIZE_WRAP_CONTENT || szChild.cy == SIZE_WRAP_CONTENT)
				{
					int nWid = szChild.cx, nHei = szChild.cy;

					CSize szCalc = pChild->GetDesiredSize(nWid,nHei);
					if(szChild.cx == SIZE_WRAP_CONTENT) 
					{
						szChild.cx = szCalc.cx;
						szChild.cx += pLinearLayoutParam->extend_left.toPixelSize(nScale) + pLinearLayoutParam->extend_right.toPixelSize(nScale);
					}
					if(szChild.cy == SIZE_WRAP_CONTENT) 
					{
						szChild.cy = szCalc.cy;
						szChild.cy += pLinearLayoutParam->extend_top.toPixelSize(nScale) + pLinearLayoutParam->extend_bottom.toPixelSize(nScale);
					}
				}
				fWeight += pLinearLayoutParam->weight;

				ppChilds[iChild]=pChild;
				pSize [iChild] = szChild;
				pChild = pParent->GetNextLayoutChild(pChild);
				iChild++;
			}
			nChilds = iChild;
        }

		int size = m_orientation == Vert? nHeight:nWidth;
		int nInterval = m_interval.toPixelSize(pParent->GetScale());
		if(!SLayoutSize::fequal(fWeight,0.0f)
			&& size != SIZE_WRAP_CONTENT )
		{//assign weight for elements. and calc size of other orientation again.
			int offset = 0;
			for(int i=0;i<nChilds;i++)
			{
				offset += m_orientation == Vert?pSize[i].cy:pSize[i].cx;
			}
			offset += nInterval * (nChilds-1);
			if(offset < size)
			{//assign size by weight
				int nRemain = size - offset;						

				ORIENTATION orienOther = m_orientation == Vert?Horz:Vert;
				for(int iChild = 0;iChild < nChilds;iChild ++)
				{
					if (SLayoutSize::fequal(fWeight, 0.0f))
						break;
					SWindow *pChild = ppChilds[iChild];
					SLinearLayoutParam *pLinearLayoutParam = pChild->GetLayoutParamT<SLinearLayoutParam>();
					int nScale = pChild->GetScale();
					if(pLinearLayoutParam->weight > 0.0f)
					{
						int extra = int(nRemain*pLinearLayoutParam->weight / fWeight + 0.5f);
						LONG & szChild = m_orientation == Vert? pSize[iChild].cy:pSize[iChild].cx;
						szChild += extra;
						nRemain -= extra;
						fWeight -= pLinearLayoutParam->weight;

						if(!pLinearLayoutParam->IsSpecifiedSize(orienOther))
						{//As pChild->GetDesiredSize may use layout param to get specified size, we must set it to new size.
							ILayoutParam * backup = pLinearLayoutParam->Clone();
							pLinearLayoutParam->SetSpecifiedSize(m_orientation,SLayoutSize((float)szChild,SLayoutSize::dp));
							pLinearLayoutParam->SetWrapContent(orienOther);
							int nWid = pSize[iChild].cx, nHei = pSize[iChild].cy;

							if(orienOther == Vert) 
								nHei = SIZE_WRAP_CONTENT;
							else 
								nWid = SIZE_WRAP_CONTENT;

							CSize szCalc = pChild->GetDesiredSize(nWid,nHei);
							if(orienOther == Vert)
							{
								szCalc.cy += pLinearLayoutParam->extend_top.toPixelSize(nScale) + pLinearLayoutParam->extend_bottom.toPixelSize(nScale);
								pSize[iChild].cy = szCalc.cy;
							}
							else
							{
								szCalc.cx += pLinearLayoutParam->extend_left.toPixelSize(nScale) + pLinearLayoutParam->extend_right.toPixelSize(nScale);
								pSize[iChild].cx = szCalc.cx;
							}
							pChild->SetLayoutParam(backup);
							backup->Release();
						}
					}
				}
			}
		}

		CSize szRet;
		for(int i=0;i<nChilds;i++)
		{
			if(m_orientation == Horz)
			{
				szRet.cy = smax(szRet.cy,pSize[i].cy);
				szRet.cx += pSize[i].cx;
			}else
			{
				szRet.cx = smax(szRet.cx,pSize[i].cx);
				szRet.cy += pSize[i].cy;
			}
		}
		//add intervals
		if (m_orientation == Horz)
		{
			szRet.cx += nInterval * (nChilds-1);
		}
		else
		{
			szRet.cy += nInterval * (nChilds - 1);
		}
		delete []pSize;
		delete []ppChilds;
		return szRet;
	}

	bool SLinearLayout::IsParamAcceptable(ILayoutParam *pLayoutParam) const
	{
		return !!pLayoutParam->IsClass(SLinearLayoutParam::GetClassName());
	}

	ILayoutParam * SLinearLayout::CreateLayoutParam() const
	{
		return new SLinearLayoutParam();
	}

}
