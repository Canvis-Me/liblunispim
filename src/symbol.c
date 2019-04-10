/**	处理中文、英文符号模块
 */
#include <config.h>
#include <context.h>
#include <utility.h>
#include <symbol.h>
#include <vc.h>
#include <share_segment.h>
#include <utf16char.h>
#pragma data_seg(HYPIM_SHARED_SEGMENT)

//全角英文字符
TCHAR full_shape_string[][4] =
{
    TEXT("ａ"), TEXT("ｂ"), TEXT("ｃ"), TEXT("ｄ"), TEXT("ｅ"), TEXT("ｆ"), TEXT("ｇ"), TEXT("ｈ"), TEXT("ｉ"), TEXT("ｊ"), TEXT("ｋ"), TEXT("ｌ"), TEXT("ｍ"), TEXT("ｎ"), TEXT("ｏ"), TEXT("ｐ"), TEXT("ｑ"), TEXT("ｒ"), TEXT("ｓ"), TEXT("ｔ"), TEXT("ｕ"), TEXT("ｖ"), TEXT("ｗ"), TEXT("ｘ"), TEXT("ｙ"), TEXT("ｚ"),
    TEXT("Ａ"), TEXT("Ｂ"), TEXT("Ｃ"), TEXT("Ｄ"), TEXT("Ｅ"), TEXT("Ｆ"), TEXT("Ｇ"), TEXT("Ｈ"), TEXT("Ｉ"), TEXT("Ｊ"), TEXT("Ｋ"), TEXT("Ｌ"), TEXT("Ｍ"), TEXT("Ｎ"), TEXT("Ｏ"), TEXT("Ｐ"), TEXT("Ｑ"), TEXT("Ｒ"), TEXT("Ｓ"), TEXT("Ｔ"), TEXT("Ｕ"), TEXT("Ｖ"), TEXT("Ｗ"), TEXT("Ｘ"), TEXT("Ｙ"), TEXT("Ｚ"),
    TEXT("０"), TEXT("１"), TEXT("２"), TEXT("３"), TEXT("４"), TEXT("５"), TEXT("６"), TEXT("７"), TEXT("８"), TEXT("９"), TEXT("，"), TEXT("．"), TEXT("／"), TEXT("＼"), TEXT("｜"), TEXT("）"), TEXT("（"), TEXT("＊"), TEXT("＆"), TEXT("＾"), TEXT("％"), TEXT("＄"), TEXT("＠"), TEXT("＂"), TEXT("："), TEXT("；"),
    TEXT("＇"), TEXT("｀"), TEXT("￣"), TEXT("＝"), TEXT("＋"), TEXT("－"), TEXT("＿"), TEXT(" "),
};

//单引号
TCHAR quotation1[2][SYMBOL_LENGTH] = { TEXT("‘"), TEXT("’")};

//双引号
TCHAR quotation2[2][SYMBOL_LENGTH] = { TEXT("“"), TEXT("”")};
//int  q2_index = 0;

#pragma	data_seg()

/**	判断是否为符号字符
 */
int IsSymbolChar(TCHAR ch)
{
    int i;

    for (i = 0; i < SYMBOL_NUMBER; i++)
        if (share_segment->symbol_table[i].english_ch == ch)
            return 1;

    return 0;
}

/**	更新英文、中文符号表
 */
void UpdateSymbol(TCHAR *english_symbol, TCHAR *chinese_symbol)
{
    int i;

    //判断是否越界
    if (utf16_strlen(english_symbol) > 2 || utf16_strlen(chinese_symbol) > SYMBOL_LENGTH)
        return;

    //寻找符号并更新
    for (i = 2; i < SYMBOL_NUMBER; i++)		//跳过头两个
        if (share_segment->symbol_table[i].english_ch == english_symbol[0])
        {
            utf16_strcpy(share_segment->symbol_table[i].chinese_symbol, chinese_symbol);
            return;
        }
}

/**	装载符号定义文件
 */
int LoadSymbolData(const char *file_name)
{
    TCHAR buffer[10 * 1024];			//10K的buffer
    TCHAR line[0x100];
    TCHAR *english_symbol, *chinese_symbol;
    int  char_count;
    int  length, index;
    int  i;
    TCHAR *saveptr;
    if (share_segment->symbol_loaded)
        return 1;

    //Copy Default symbol table
    for (i = 0; i < SYMBOL_NUMBER; i++)
        utf16_strcpy(share_segment->symbol_table[i].chinese_symbol, share_segment->symbol_table[i].default_chinese_symbol);

    if (!(length = LoadFromFile(file_name, buffer, _SizeOf(buffer))))
        return 0;

    index  = 1;
    length = length / sizeof(TCHAR);

    while(index < length)
    {
        //得到一行数据
        char_count = 0;
        while(char_count < _SizeOf(line) - 1 && index < length)
        {
            line[char_count++] = buffer[index++];

            if (buffer[index - 1] == '\n')
                break;				//遇到回车结束
        }

        line[char_count] = 0;		//得到一行数据

        //除掉首尾的空白符号
        TrimString(line);

        if (line[0] == '/' && line[1]=='/')			//注释行，跳过
            continue;

        english_symbol = utf16_strtok(line, TEXT(" "), &saveptr);
        chinese_symbol = utf16_strtok(0, TEXT(" "), &saveptr);

        if (!english_symbol || !chinese_symbol)
            continue;
        //TrimString(english_symbol);
        //TrimString(chinese_symbol);

        UpdateSymbol(english_symbol, chinese_symbol);
    }

    share_segment->symbol_loaded = 1;

    return 1;
}

int FreeSymbolData()
{
    share_segment->symbol_loaded = 0;
    return 1;
}

/**	获得当前的符号
 */
const TCHAR *GetSymbol(PIMCONTEXT *context, TCHAR ch)
{
    int i;

    for (i = 0; i < SYMBOL_NUMBER; i++)
        if (ch == share_segment->symbol_table[i].english_ch)
            break;
    if(i == SYMBOL_NUMBER){
        //全角符号输出优先
        if(!(pim_config->hz_option & HZ_SYMBOL_HALFSHAPE)){
            if (ch == ' '){
                return TEXT("　");

            }else if (ch >= 'a' && ch <= 'z'){
                return full_shape_string[ch - 'a'];

            }else if (ch >= 'A' && ch <= 'Z'){
                return full_shape_string[26 + ch - 'A'];

            }else if (ch >= '0' && ch <= '9'){
                return full_shape_string[52 + ch - '0'];
            }
        }
        return 0;
    }else{
        if(!(pim_config->hz_option & HZ_SYMBOL_HALFSHAPE)){
            switch (ch) {
            case ',':
                return full_shape_string[62];
            case '.':
                return full_shape_string[63];
            case '/':
                return full_shape_string[64];
            case '\\':
                return full_shape_string[65];
            case '|':
                return full_shape_string[66];
            case ')':
                return full_shape_string[67];
            case '(':
                return full_shape_string[68];
            case '*':
                return full_shape_string[69];
            case '&':
                return full_shape_string[70];
            case '^':
                return full_shape_string[71];
            case '%':
                return full_shape_string[72];
            case '$':
                return full_shape_string[73];
            case '@':
                return full_shape_string[74];
            case '"':
                return full_shape_string[75];
            case ':':
                return full_shape_string[76];
            case ';':
                return full_shape_string[77];
            case '\'':
                return full_shape_string[78];
            case '`':
                return full_shape_string[79];
            case '~':
                return full_shape_string[80];
            case '=':
                return full_shape_string[81];
            case '+':
                return full_shape_string[82];
            case '-':
                return full_shape_string[83];
            case '_':
                return full_shape_string[84];
            }
        }

        if (!(pim_config->hz_option & HZ_SYMBOL_CHINESE) || (context->english_state == ENGLISH_STATE_INPUT))
            return share_segment->symbol_table[i].english_symbol;

        if ((context->input_mode & CHINESE_MODE) &&	(context->last_digital) &&
                ((pim_config->english_symbol_follow_number) || (pim_config->english_dot_follow_number && ch == '.')))
            return share_segment->symbol_table[i].english_symbol;

        if (i == 0)		//单引号
            return quotation1[share_segment->q1_index];

        if (i == 1)		//双引号
            return quotation2[share_segment->q2_index];
        //正常符号
        return share_segment->symbol_table[i].chinese_symbol;
    }
}

/**	输入了引号，包括单引号、双引号
 */
void CheckQuoteInput(HZ symbol)
{

    if (symbol == *(HZ*)TEXT("“"))
        share_segment->q2_index = 1;
    else if (symbol == *(HZ*)TEXT("”"))
        share_segment->q2_index = 0;
    else if (symbol == *(HZ*)TEXT("‘"))
        share_segment->q1_index = 1;
    else if (symbol == *(HZ*)TEXT("’"))
        share_segment->q1_index = 0;
}

int IsURLInput(const TCHAR *input)
{
    static TCHAR *direct_inputs[] =
    {
        TEXT("www."),	TEXT("ftp."),	TEXT("bbs."),	TEXT("forum."),		TEXT("mail."),	TEXT("blog."),
        TEXT("ftp:"),	TEXT("http:"),	TEXT("https:"),	TEXT("mailto:"),	TEXT("file:"),
    };

    int i;

    for (i = 0; i < sizeof(direct_inputs) / _SizeOf(direct_inputs[0]) / sizeof(TCHAR); i++)
        if (!utf16_strncmp(direct_inputs[i], input, utf16_strlen(direct_inputs[i])))
            return 1;

    return 0;
}

/** 获得全角字符串
 */
void GetFullShapes(const TCHAR *src, TCHAR *dest, int dest_len)
{
    int i;
    TCHAR c[2] = {0, 0};

    if (IsURLInput(src))
        return;

    for (i = 0; i < (int)utf16_strlen(src); i++)
    {

        if (src[i] == ' ')
            _tcscat_s(dest, dest_len, TEXT("　"));
        else if (src[i] >= 'a' && src[i] <= 'z')
            _tcscat_s(dest, dest_len, full_shape_string[src[i] - 'a']);
        else if (src[i] >= 'A' && src[i] <= 'Z')
            _tcscat_s(dest, dest_len, full_shape_string[26 + src[i] - 'A']);
        else if (src[i] >= '0' && src[i] <= '9')
            _tcscat_s(dest, dest_len, full_shape_string[52 + src[i] - '0']);
        else
        {
            switch (src[i]) {
            case ',':
                _tcscat_s(dest, dest_len, full_shape_string[62]);
                break;
            case '.':
                _tcscat_s(dest, dest_len, full_shape_string[63]);
                break;
            case '/':
                _tcscat_s(dest, dest_len, full_shape_string[64]);
                break;
            case '\\':
                _tcscat_s(dest, dest_len, full_shape_string[65]);
                break;
            case '|':
                _tcscat_s(dest, dest_len, full_shape_string[66]);
                break;
            case ')':
                _tcscat_s(dest, dest_len, full_shape_string[67]);
                break;
            case '(':
                _tcscat_s(dest, dest_len, full_shape_string[68]);
                break;
            case '*':
                _tcscat_s(dest, dest_len, full_shape_string[69]);
                break;
            case '&':
                _tcscat_s(dest, dest_len, full_shape_string[70]);
                break;
            case '^':
                _tcscat_s(dest, dest_len, full_shape_string[71]);
                break;
            case '%':
                _tcscat_s(dest, dest_len, full_shape_string[72]);
                break;
            case '$':
                _tcscat_s(dest, dest_len, full_shape_string[73]);
                break;
            case '@':
                _tcscat_s(dest, dest_len, full_shape_string[74]);
                break;
            case '"':
                _tcscat_s(dest, dest_len, full_shape_string[75]);
                break;
            case ':':
                _tcscat_s(dest, dest_len, full_shape_string[76]);
                break;
            case ';':
                _tcscat_s(dest, dest_len, full_shape_string[77]);
                break;
            case '\'':
                _tcscat_s(dest, dest_len, full_shape_string[78]);
                break;
            case '`':
                _tcscat_s(dest, dest_len, full_shape_string[79]);
                break;
            case '~':
                _tcscat_s(dest, dest_len, full_shape_string[80]);
                break;
            case '=':
                _tcscat_s(dest, dest_len, full_shape_string[81]);
                break;
            case '+':
                _tcscat_s(dest, dest_len, full_shape_string[82]);
                break;
            case '-':
                _tcscat_s(dest, dest_len, full_shape_string[83]);
                break;
            case '_':
                _tcscat_s(dest, dest_len, full_shape_string[84]);
                break;
            default:
                c[0] = src[i];
                _tcscat_s(dest, dest_len, c);
            }

        }
    }
} 
