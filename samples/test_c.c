struct Point2D {
    int x;
    int y;
};

struct Point2D f()
{
    struct Point2D xy = { 10, 20 };
    return xy;
}

int main()
{
    struct Point2D xy = f();
    return xy.x;
}
// struct Point2D {
//     double x;
//     double y;
// };

// struct Point2D f()
// {
//     struct Point2D xy = { 10.0, 20.0 };
//     return xy;
// }

// struct Point2D f()
// {
//     struct Point2D point = { 10.0, 20.0 };
//     return point;
// }
