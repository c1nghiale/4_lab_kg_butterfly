varying vec2 texCoord;

//константы цветов
const vec3 COL_BG   = vec3(1.0);
const vec3 COL_WING = vec3(1.0, 0.6, 0.1);
const vec3 COL_DARK = vec3(0.15, 0.08, 0.02);

//утилиты
float distToSeg(vec2 p, vec2 a, vec2 b)
{
    vec2 pa = p - a;
    vec2 ba = b - a;

    float h = clamp(
        dot(pa, ba) / dot(ba, ba),
        0.0,
        1.0
    );

    return length(pa - ba * h);
}

float ellipse(vec2 p, vec2 c, vec2 r)
{
    vec2 d = (p - c) / r;

    return step(dot(d, d), 1.0);
}

float lineMask(vec2 p, vec2 a, vec2 b, float w)
{
    return step(
        distToSeg(p, a, b),
        w
    );
}

//----------------------------------
//
vec2 bezier( vec2 a, vec2 b, vec2 c, float t){
    float s = 1.0 - t;
    return  s*s*a +2.0*s*t*b +t*t*c;
}

// отрисовка крыла
float wingContour(vec2 p)
{
    vec2 A = vec2(0.50, 0.55);

    vec2 B = vec2(0.70, 0.95);
    vec2 C = vec2(0.92, 0.68);

    vec2 D = vec2(0.86, 0.22);
    vec2 E = vec2(0.55, 0.35);

    float d = 1e5;

    //верхняя арка крыла
    vec2 prev = A;

    for(int i=1; i<=24; i++)
    {
        float t = float(i)/24.0;

        vec2 cur = bezier(
            A, B, C, t);

        d = min(d,
            distToSeg(p, prev, cur));

        prev = cur;
    }


    //нижняя арка крыла
    prev = C;

    for(int i=1; i<=24; i++)
    {
        float t = float(i)/24.0;

        vec2 cur = bezier(
            C, D, E, t);

        d = min(d,
            distToSeg(p, prev, cur));

        prev = cur;
    }

    //замыкаем
    d = min(d,
        distToSeg(p, E, A));

    return d;
}

// проверка находится ли точка внутри крыла
float isInsideWing(vec2 p)
{
    vec2 A = vec2(0.50, 0.55);
    vec2 B = vec2(0.70, 0.95);
    vec2 C = vec2(0.92, 0.68);
    vec2 D = vec2(0.86, 0.22);
    vec2 E = vec2(0.55, 0.35);

    // аппроксимируем крыло многоугольником
    int numPoints = 50;
    vec2 points[50];

    // верхняя арка
    for(int i = 0; i <= 24; i++)
    {
        float t = float(i)/24.0;
        points[i] = bezier(A, B, C, t);
    }

    // нижняя арка
    for(int i = 25; i <= 49; i++)
    {
        float t = float(i-25)/24.0;
        points[i] = bezier(C, D, E, t);
    }

    // проверка точки внутри многоугольника (алгоритм четности)
    bool inside = false;
    for(int i = 0, j = 49; i < 50; j = i++)
    {
        vec2 pi = points[i];
        vec2 pj = points[j];

        if(((pi.y > p.y) != (pj.y > p.y)) &&
           (p.x < (pj.x - pi.x) * (p.y - pi.y) / (pj.y - pi.y) + pi.x))
        {
            inside = !inside;
        }
    }

    return inside ? 1.0 : 0.0;
}

//крыло (полная заливка)
float wingMask(vec2 p)
{
    return isInsideWing(p);
}

//граница крыла
float borderMask(vec2 p)
{
    float outer = wingMask(p);

    vec2 center = vec2(0.70, 0.58);

    float inner =
        wingMask(
            (p - center) * 1.08 + center
        );

    return clamp(
        outer - inner,
        0.0,
        1.0
    );
}

//прожилки
float veinMask(vec2 p)
{
    vec2 base = vec2(0.52, 0.55);

    float v = 0.0;

    v += lineMask(
        p,
        base,
        vec2(0.78, 0.88),
        0.004
    );

    v += lineMask(
        p,
        base,
        vec2(0.90, 0.70),
        0.004
    );

    v += lineMask(
        p,
        base,
        vec2(0.88, 0.52),
        0.004
    );

    v += lineMask(
        p,
        base,
        vec2(0.80, 0.34),
        0.004
    );

    return clamp(v, 0.0, 1.0);
}

//точки для крыльев
float dotMask(vec2 p)
{
    float d = 0.0;

    d += ellipse(
        p,
        vec2(0.78, 0.75),
        vec2(0.030)
    );

    d += ellipse(
        p,
        vec2(0.82, 0.58),
        vec2(0.024)
    );

    return clamp(d, 0.0, 1.0);
}

//маска для тела
float bodyMask(vec2 p)
{
    float body =
        ellipse(
            p,
            vec2(0.5, 0.55),
            vec2(0.025, 0.20)
        );

    float head =
        ellipse(
            p,
            vec2(0.5, 0.78),
            vec2(0.03)
        );

    return clamp(
        body + head,
        0.0,
        1.0
    );
}

//собираем бабочку
vec3 composeButterfly(
    vec2 p,
    vec2 ps)
{
    float wing   = wingMask(ps);
    float border = borderMask(ps);
    float veins  = veinMask(ps) * wing;
    float dots   = dotMask(ps) * wing;
    float body   = bodyMask(p);

    vec3 col = COL_BG;

    col = mix(col, COL_WING, wing);
    col = mix(col, COL_DARK, border);
    col = mix(col, COL_DARK, veins);
    col = mix(col, COL_DARK, dots);
    col = mix(col, COL_DARK, body);

    return col;
}

void main()
{
    vec2 uv = texCoord;

    // визуально бабочка повернется против часовой
    vec2 p = vec2(
        0.5 - uv.y,
        1.0 - uv.x
    );

    vec2 ps =
        vec2(abs(p.x - 0.5) + 0.5, p.y);

    vec3 col =
        composeButterfly(p, ps);

    gl_FragColor = vec4(col, 1.0);
}
