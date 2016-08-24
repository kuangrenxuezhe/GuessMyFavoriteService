class CXmlParser
{
public:
    //FUNC  得到Tag pair的前后位置
    int GetTagPairPos(const char *pParseBeg, const char *pParseEnd, const char *pTagName, char *&pTagBeg, char *&pTagEnd)
    {
        if (pParseBeg == NULL || pParseEnd == NULL || pTagName == NULL)
        {
            return -10;
        }
        pTagBeg = NULL;
        pTagEnd = NULL;
        
        int iTagLen = strlen(pTagName);
        char *p = (char *)pParseBeg;
        while (p < pParseEnd)
        {
            if (*p < 0)
            {
                p += 2;
                if (p > pParseEnd)
                {
                    return -100;
                }
            }
            else
            {
                if (*p == '<')
                {
                    if (memcmp(p+1, pTagName, iTagLen) == 0
                        && *(p+1+iTagLen) == '>')
                    {//是起始标记
                        p += 1+iTagLen+1;
                        
                        pTagBeg = p;
                    }
                    else if (pTagBeg != NULL
                        && *(p+1) == '/'
                        && memcmp(p+2, pTagName, iTagLen) == 0
                        && *(p+2+iTagLen) == '>')
                    {
                        pTagEnd = p;
                        
                        if (memcmp(pTagBeg, "<![CDATA[", 9) == 0
                            && memcmp(pTagEnd-3, "]]>", 3) == 0)
                        {
                            pTagBeg += 9;
                            pTagEnd -= 3;
                        }
                        return 0;
                    }
                    else
                    {
                        ++p;
                    }
                }
                else
                {
                    ++p;
                }
            }
        }
        
        return 0;
    }
protected:
private:
};
