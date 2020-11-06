#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <locale>
#include <codecvt>
#include <iomanip>
#include <chrono>
#include <algorithm>

using namespace std;
const string delimeter = ",";
const size_t ZIP_LENGTH = 5;
vector<string> Split(string& data, const string& del);

void FormatTime(string& timestr)
{
    std::tm tm = {};
    std::stringstream ss(timestr);
    ss >> std::get_time(&tm, "%m/%d/%y %H:%M:%S %p");
    tm.tm_isdst = -1;
    time_t t = mktime(&tm);
    auto time = std::chrono::system_clock::from_time_t(t);
    time += chrono::hours(3);
    t = chrono::system_clock::to_time_t(time);

    stringstream sstime;
    sstime << std::put_time(localtime(&t), "%Y-%m-%d %H:%M:%S");
    timestr = sstime.str();

}

void FormatZipCode(string& zipstr)
{
    while( zipstr.length() < ZIP_LENGTH)
        zipstr.insert(0, "0");
}

void FormatName(string& namestr)
{
    std::transform(namestr.begin(), namestr.end(), namestr.begin(), ::toupper );
}

float ConvertToFloatTime(string& ftime )
{
    int h = 0;
    int m = 0;
    int s = 0;
    int ms = 0;

    sscanf(ftime.c_str(), "%d:%d:%d.%d", &h, &m, &s, &ms );
    float fval = h * 3600.0f + m * 60.0f + s + ms / 1000.0f;

    //erase the floatting point error
    ftime = to_string(fval);
    ftime.erase(ftime.find(".")+4);

    return fval;
}

void FloatToString(string& fstr, float val)
{
    fstr = to_string(val);
    fstr.erase(fstr.find(".")+4);
}

void ValidateUnicode( string& unicode )
{
    wchar_t invalid = L'\uFFFD';

    int i,f_size=unicode.size();
    unsigned char c,c2,c3,c4;
    string to;
    to.reserve(f_size);

    for(i=0 ; i<f_size ; i++)
    {
        c=(unsigned char)unicode[i];
        if(c<32){//control char
            if(c==9 || c==10 || c==13){//allow only \t \n \r
                to.append(1,c);
            }
            continue;
        }else if(c<127){//normal ASCII
            to.append(1,c);
            continue;
        }else if(c<160){//control char (nothing should be defined here either ASCI, ISO_8859-1 or UTF8, so skipping)
            if(c2==128){//fix microsoft mess, add euro
                to.append(1,226);
                to.append(1,130);
                to.append(1,172);
            }
            if(c2==133){//fix IBM mess, add NEL = \n\r
                to.append(1,10);
                to.append(1,13);
            }
            continue;
        }else if(c<192){//invalid for UTF8, converting ASCII
            to.append(1,(unsigned char)194);
            to.append(1,c);
            continue;
        }else if(c<194){//invalid for UTF8, converting ASCII
            to.append(1,(unsigned char)195);
            to.append(1,c-64);
            continue;
        }else if(c<224 && i+1<f_size){//possibly 2byte UTF8
            c2=(unsigned char)unicode[i+1];
            if(c2>127 && c2<192){//valid 2byte UTF8
                if(c==194 && c2<160){//control char, skipping
                    ;
                }else{
                    to.append(1,c);
                    to.append(1,c2);
                }
                i++;
                continue;
            }
        }else if(c<240 && i+2<f_size){//possibly 3byte UTF8
            c2=(unsigned char)unicode[i+1];
            c3=(unsigned char)unicode[i+2];
            if(c2>127 && c2<192 && c3>127 && c3<192){//valid 3byte UTF8
                to.append(1,c);
                to.append(1,c2);
                to.append(1,c3);
                i+=2;
                continue;
            }
        }else if(c<245 && i+3<f_size){//possibly 4byte UTF8
            c2=(unsigned char)unicode[i+1];
            c3=(unsigned char)unicode[i+2];
            c4=(unsigned char)unicode[i+3];
            if(c2>127 && c2<192 && c3>127 && c3<192 && c4>127 && c4<192){//valid 4byte UTF8
                to.append(1,c);
                to.append(1,c2);
                to.append(1,c3);
                to.append(1,c4);
                i+=3;
                continue;
            }
        }
        //invalid UTF8, converting ASCII (c>245 || string too short for multi-byte))
        to.append(1,(unsigned char)0xFF);
        to.append(1,0xFD);
    }

    unicode = to;
}

int main(int argc, char *argv[])
{
    string filename;
    ifstream fin;
    if( argc > 1 )
    {
        filename = argv[1];
        fin.open(filename);
        if(!fin.is_open() )
        {
            cout << "Invalid file " << filename << endl;
            return 0;
        }
    }

    string data;
    string header = "";
    int count = 0;
    string output;
    while(getline(fin.is_open() ? fin : cin, data))
    {
        ++count;
        if( count == 1 )
        {
            header = data;
            cout << header << endl;
            continue;
        }

        auto splitstrs = Split(data, delimeter);

        output.clear();

        //time is first value
        FormatTime(splitstrs[0]);
        output += splitstrs[0];
        output += delimeter;

        ValidateUnicode(splitstrs[1]); //address
        output += splitstrs[1];
        output += delimeter;

        FormatZipCode(splitstrs[2]);
        output += splitstrs[2];
        output += delimeter;

        FormatName(splitstrs[3]);
        output += splitstrs[3];
        output += delimeter;

        //convert floats and set value
        float val1 = ConvertToFloatTime(splitstrs[4]); //foo
        float val2 = ConvertToFloatTime(splitstrs[5]); //bar
        FloatToString(splitstrs[6], val1 + val2);

        output += splitstrs[4];
        output += delimeter;
        output += splitstrs[5];
        output += delimeter;
        output += splitstrs[6];
        output += delimeter;

        ValidateUnicode(splitstrs[7]); //notes
        output += splitstrs[7];

        cout << output << endl;

    }

    return 0;
}

vector<string> Split(string& data, const string& del)
{
    vector<string> retval;
    if( data.empty() )
        return retval;


    size_t start = 0;
    size_t pos = data.find(del, start);
    while( pos != string::npos  )
    {
        string token = data.substr(start, pos - start);
        retval.emplace_back(token);
        start = pos + 1;

        //peek ahead for quote
        if( data[start] == '"' )
        {
            start++;
            pos = data.find('"', start);
        }
        else
        {
            if(data[start] == ',')
                ++start;

            pos = data.find(del, start);
        }

    }

    string token = data.substr(start);
    retval.emplace_back(token);

    return retval;
}
