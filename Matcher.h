#ifndef _MATCHER_HEADER
#define _MATCHER_HEADER

#include <vector>
#include <queue>
#include <iterator>
#include <algorithm>
#include <utility>

template<typename Elem>
class KmpMatcher {
public:
    typedef std::basic_string<Elem> pattern_type;

protected:
    typedef std::vector<int> Fn;

public:
    explicit
        KmpMatcher(const pattern_type& s,bool sense = true)
        : ptn(s),f(s.length()),s(sense) {
            construct();
    }

    template<typename BiDirectIter>
    BiDirectIter find( BiDirectIter beg,BiDirectIter end ) const {
        pattern_type::size_type k = 0;  // state - k, pos - k
        while( k < ptn.length() && beg != end ) {
            if( eq(*beg,ptn[k]) ) {
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
        if(k == ptn.length()) {
            advance(beg,-ptn.length());
        }
        return beg;
    }

    template<typename BiDirectIter>
    size_t count( BiDirectIter beg,BiDirectIter end ) const {
        size_t cn = 0;
        beg = find(beg,end);
        while(beg != end) {
            ++cn;
            beg = find(++beg,end);
        }
        return cn;
    }

    const pattern_type& get_pattern() const {
        return ptn;
    }

    void sense_case(bool sense) {
        s = sense;
    }

    bool sense_case() const {
        return s;
    }

protected:
    void construct() {
        f[0] = -1;  // s = 0
        for(pattern_type::size_type i = 0;i + 1 < ptn.length();++i) {
            int k = f[i];   // s = i
            // 查找能够最长的前缀
            while( k >= 0 && !eq(ptn[i],ptn[k]) )
                k = f[k];
            // 每次设置下一个状态的实效函数
            f[i+1] = k + 1; // s = i + 1
        }
        // 优化
        for(Fn::size_type i = 1;i < f.size();++i) {
            int k = f[i];
            while( k >= 0 && eq(ptn[i],ptn[k]) ) // ptn[i]下一个接受字符
                k = f[k];
            f[i] = k;
        }
    }

    bool eq(const Elem& e1,const Elem& e2) const {
        if(s) {
            return e1 == e2;
        }
        return toupper(e1) == toupper(e2);
    }

private:
    pattern_type ptn;
    Fn f;
    bool s;
};

template<typename Elem>
class AcKmpMatcher {
public:
    typedef std::basic_string<Elem> pattern_type;
    typedef std::vector<pattern_type*> pattern_array;
    typedef typename pattern_array::size_type position_type;

protected:
    struct Node;
    typedef std::vector<Node> node_array;
    typedef typename pattern_array::iterator pattern_iterator;
    enum { CHLD_MASK = 0x80000000 };

public:
    template<typename InputIter>
    AcKmpMatcher(InputIter beg,InputIter end) {
        while(beg != end) {
            pattern_type* p = new pattern_type(*beg);
            pa.push_back(p);
            ++beg;
        }
        construct();
    }

    template<typename BiDirectIter>
    std::pair<BiDirectIter,position_type> find( BiDirectIter beg,BiDirectIter end ) const {
        int s = 0;
        while( na[s].chld >= 0 && beg != end ) {
            int t = find_child(s,*beg);
            if( t < 0 ) {
                s = na[s].fn;
                if( s < 0 ) {
                    ++beg;
                    s = 0;
                }
            }
            else {
                s = t;
                ++beg;
            }
        }
        position_type pos = -1;
        if(na[s].chld < 0) {
            pos = na[s].chld ^ CHLD_MASK;
            advance( beg,-pa[pos]->length() );
        }
        return make_pair(beg,pos);
    }

    template<typename BiDirectIter>
    size_t count( BiDirectIter beg,BiDirectIter end ) const {
        size_t cn = 0;
        std::pair<BiDirectIter,position_type> result = find(beg,end);
        while(result.first != end) {
            ++cn;
            result = find(++result.first,end);
        }
        return cn;
    }

    const pattern_type* get_pattern(position_type pos) const {
        if(pos >= 0 && pos < pa.size()) {
            return pa[pos];
        }
        return NULL;
    }

    ~AcKmpMatcher() {
        for(pattern_array::iterator iter(pa.begin());iter != pa.end();++iter) {
            delete *iter;
        }
    }

protected:
    struct cmp {
        bool operator () (pattern_type* l,pattern_type* r) const {
            return *l < *r;
        }
    };

    struct Node {
        int bro;
        int chld;
        int fn;
        Elem c;
    };

    // 计算实际需要的结点个数，分治算法实现
    size_t do_count_states(pattern_iterator first,pattern_iterator last,size_t i) const {
        pattern_iterator mid(first);
        ++mid;
        if(mid == last)
            return (*first)->length()-i;
        size_t cn = 0;
        while(mid < last) {
            if(i == (*mid)->length()) { // 就是next_string[i] = '\0'，分点
                cn += 1 + do_count_states(first,mid,i+1);
                while(mid != last && i == (*mid)->length())
                    ++mid;
                first = mid;
            }
            else if( (**first)[i] != (**mid)[i] ) { // 就是this_string[i] != next_string[i]，分点
                cn += 1 + do_count_states(first,mid,i+1);
                first = mid;
            }
            ++mid;
        }
        if(mid == last) { // 就是last_string with no next_string，分点
            cn += 1 + do_count_states(first,mid,i+1);
        }
        return cn;
    }

    size_t count_states(pattern_iterator first,pattern_iterator last) const {
        return 1 + do_count_states(first,last,0);
    }

    int find_child(int r,Elem c) const {
        int chld = na[r].chld;
        while(chld >= 0 && na[chld].c != c) {
            chld = na[chld].bro;
        }
        return chld;
    }

    void build_trie() {
        int pos = 0;
        int r = pos++;
        na[r].bro = -1;
        na[r].chld = -1;
        na[r].fn = -1;
        na[r].c = Elem();

        for(int i = 0;i < pa.size();i++) {
            pattern_type& ptn = *pa[i];
            pattern_type::iterator iter(ptn.begin());

            r = 0;
            int chld;
            while(iter != ptn.end()) { // 匹配前缀与前缀
                chld = find_child(r,*iter);
                if(chld < 0) {
                    break;
                }
                r = chld;
                ++iter;
            }
            while(iter != ptn.end()) { // 添加后缀
                chld = pos++;
                na[chld].c = *iter;
                na[chld].chld = -1;
                na[chld].bro = na[r].chld;
                na[r].chld = chld;
                r = chld;
                ++iter;
            }
            na[r].chld = i ^ CHLD_MASK; // 计算对应的模式串编号
        }
    }

    void do_fill_fn(int s,int t) {
        int u = na[s].fn;
        int v = -1;
        Elem c = na[t].c;
        while(u >= 0) {
            v = find_child(u,c);
            if(v >= 0) {
                break;
            }
            u = na[u].fn; // 递归寻找合适的最长的前缀
        }
        na[t].fn = v;
    }

    void fill_fn() { // 广度遍历填充失效函数
        std::queue<int> q;
        q.push(0);
        na[0].fn = -1;
        while(!q.empty()) {
            int s = q.front();
            q.pop();
            for(int t = na[s].chld;t >= 0;t = na[t].bro) {
                q.push(t);
                do_fill_fn(s,t);
            }
        }
    }

    void construct() {
        sort(pa.begin(),pa.end(),cmp());
        size_t cn = count_states(pa.begin(),pa.end());
        na.resize(cn);
        build_trie();
        fill_fn();
    }

private:
    pattern_array pa;
    node_array na;
};

#endif