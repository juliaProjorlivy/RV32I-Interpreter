struct Fib
{
    int a;
    int b;
};
int fib(int n, struct Fib pair)
{
    if (n == 0) return pair.a;
    else if(n == 1) return pair.b;
    else
    {
        struct Fib out = {pair.b, pair.a + pair.b};
        return fib(n - 1, out);
    };
}

int main()
{
    int n = 5;
    struct Fib initial_pair = {0, 1};
    int result = fib(n, initial_pair);
    return result;
}

