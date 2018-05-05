#include <iostream>
#include <vector>
#include <string>
#include <iterator>
#include <locale>
#include "Matcher.h"
#include <queue>

using namespace std;

int test1()
{
    cout<<"Test KMP:"<<endl;

    KmpMatcher<char> m("mAiN");

    char* p = "#include <iostream> using namespace std;int main();int main(){return 0;}";

    string s(p);
    cout<<"string : "<<s<<endl;

    KmpMatcher<char>::pattern_type::iterator iter;
    m.sense_case(false);
    iter = m.find(s.begin(),s.end());
    if(iter != s.end()) {
        cout<<"position: "<<iter-s.begin()<<" :";
        copy( iter,s.end(),ostream_iterator<char>(cout) );
        cout<<endl;
    }

    cout<<"total match: "<<m.count(s.begin(),s.end())<<endl;

    return 0;
}

int test2()
{
    cout<<"Test ACKMP:"<<endl;
    char* pp[] = {
        "hers",
        "his",
        "she"
    };
    char* p = "This is a test.She shers hers this..";
    string s(p);
    cout<<"string : "<<s<<endl;

    AcKmpMatcher<char> acm(pp,pp+sizeof(pp)/sizeof(pp[0]));
    pair<string::iterator,AcKmpMatcher<char>::position_type> result;
    result = acm.find(s.begin(),s.end());
    while(result.first != s.end()) {
        cout<<"position: "<<result.first-s.begin()<<" : "<<*acm.get_pattern(result.second)<<" : ";
        copy( result.first,s.end(),ostream_iterator<char>(cout) );
        ++result.first;
        result = acm.find(result.first,s.end());
        cout<<endl;
    }
    cout<<"total match: "<<acm.count(s.begin(),s.end())<<endl;
    return 0;
}

int main() {
    test1();
    cout<<endl;
    test2();
    return 0;
}


/*

template<typename Elem,typename Traits = char_traits<Elem> >
class Equal {
public:
    Equal()
        : loc() {
            ptype = &use_facet<ctype<Elem> >(&loc);
    }
    Equal(const locale& l) 
        : loc(l){
        ptype = &use_facet<ctype<Elem> >(&loc);
    }

    bool eq(const Elem& e1,const Elem& e2) const {
        return Traits::eq( ptype->toupper(e1),ptype->toupper(e2) );
    }

private:
    locale loc;
    ctype<Elem>* ptype;
};

template<typename Elem,
         typename PatternTraits = char_traits<Elem>,
         typename PatternEqualFunc = Equal<Elem,PatternTraits>
        >
class KmpMatcher {
public:
    typedef PatternTraits                    pattern_traits_type;
    typedef PatternEqualFunc                 pattern_equal_func;
    typedef basic_string<Elem,PatternTraits> pattern_type;

protected:
    typedef vector<int> Fn;

public:
    KmpMatcher(const pattern_type& s)
        : ptn(s),f(s.length()) {
        construct();
    }

    template<typename RandomIter>
    RandomIter find( RandomIter beg,RandomIter end ) const {
        pattern_type::size_type k = 0;  // state - k, pos - k
        while( k < ptn.length() && beg != end ) {
            if( pfunc->eq(*beg,ptn[k]) ) {
                ++k;
                ++beg;
            }
            else {
                if( f[k] < 0 ) { // 状态为负，跳过
                    k = 0;
                    ++beg;
                }
                else
                    k = f[k];
            }
        }
        if( beg != end )
            return beg - ptn.length();
        return end;
    }

    template<typename RandomIter>
    size_t count( RandomIter beg,RandomIter end ) const {
        size_t sum = 0;
        beg = find(beg,end);
        while( beg != end ) {
            ++sum;
            beg = find(++beg,end);
        }
        return sum;
    }

protected:
    void construct()
    {
        f[0] = -1;  // s = 0
        for(pattern_type::size_type i = 0;i + 1 < ptn.length();++i) {
            int k = f[i];   // s = i
            // 查找能够最长的前缀
            while( k >= 0 && !pfunc->eq(ptn[i],ptn[k]) )
                k = f[k];
            // 每次设置下一个状态的实效函数
            f[i+1] = k + 1; // s = i + 1
        }
        // 优化
        for(Fn::size_type i = 1;i < f.size();++i) {
            int k = f[i];
            while( k >= 0 && pfunc->eq(ptn[i],ptn[k]) ) // ptn[i]下一个接受字符
                k = f[k];
            f[i] = k;
        }
    }

private:
    pattern_type ptn;
    pattern_equal_func* pfunc;
    Fn f;
};

// class my_traits : protected char_traits<char> {
// public:
//     static bool eq(const char& left, const char& right) {
//         return tolower(left) == tolower(right);
//     }
// };

*/
