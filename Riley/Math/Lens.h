#pragma once

namespace Riley
{

namespace OpticalSystem
{

struct LensInterface
{
    Vector3 center;
    float radius;

    Vector3 n;
    float sa = 7.f;

    float d1;
    float flat;
    float pos;
    float w;
};

struct Ray
{
    Vector3 position, direction;
    Vector4 texcoord;
};

struct Intersection
{
    Intersection(){};
    Vector3 position;
    Vector3 norm;
    float theta;
    bool hit;
    bool inverted;
};

struct LensParameters
{
    float r; // radius
    float d; // distance to the next Interface
    float n; // refractive index of the lens
    bool f;  // check flat or curved
    float w;
    float h;
    float c;
};

struct LensDescription
{
    // Angenieux Lens
    const int angenieuxApertureId = 7;

    std::vector<LensParameters> angenieux = {{164.13f, 10.99f, 1.67510f, false, 0.5f, 52.0f, 432},
                                             {559.20f, 0.23f, 1.00000f, false, 0.5f, 52.0f, 532},

                                             {100.12f, 11.45f, 1.66890f, false, 0.5f, 48.0f, 382},
                                             {213.54f, 0.23f, 1.00000f, false, 0.5f, 48.0f, 422},

                                             {58.04f, 22.95f, 1.69131f, false, 0.5f, 36.0f, 572},

                                             {2551.10f, 2.58f, 1.67510f, false, 0.5f, 42.0f, 612},
                                             {32.39f, 30.66f, 1.00000f, false, 0.3f, 36.0f, 732},

                                             {0.0f, 10.00f, 1.00000f, true, 25.f, 7.0f, 440},

                                             {-40.42f, 2.74f, 1.69920f, false, 1.5f, 13.0f, 602},

                                             {192.98f, 27.92f, 1.62040f, false, 4.0f, 36.0f, 482},
                                             {-55.53f, 0.23f, 1.00000f, false, 0.5f, 36.0f, 662},

                                             {192.98f, 7.98f, 1.69131f, false, 0.5f, 35.0f, 332},
                                             {-225.30f, 0.23f, 1.00000f, false, 0.5f, 35.0f, 412},

                                             {175.09f, 8.48f, 1.69130f, false, 0.5f, 35.0f, 532},
                                             {-203.55f, 40.f, 1.00000f, false, 0.5f, 35.0f, 632},

                                             {0.f, 5.f, 1.00000f, true, 10.f, 5.f, 500}};

    std::vector<LensInterface> lensInterface;
    std::vector<Vector4> ghosts;

    std::vector<LensParameters> lensComponents = angenieux;
    int apertureId = angenieuxApertureId;
    int numOfGhosts = 92; // 14! / ((14-2)! * 2!) = 92

    int numOfLensComponents = (int)lensComponents.size();
    int numOfIntersections1 = numOfLensComponents + 1;
    int numOfIntersections2 = numOfLensComponents + 1;
    int numOfIntersections3 = numOfLensComponents + 1;
    float totalLensDistance = 0.0f;
    float maxIor = -1000.0f;
    float minIor = 1000.0f;
};

Vector3 reflect(Vector3 i, Vector3 n)
{
    return i - n * 2.f * i.Dot(n);
}

Vector3 refract(Vector3 i, Vector3 n, float eta)
{
    float N_dot_I = n.Dot(i);
    float k = 1.f - eta * eta * (1.f - N_dot_I * N_dot_I);
    if (k < 0.f)
        return Vector3(0.f, 0.f, 0.f);
    else
        return i * eta - n * (eta * N_dot_I + sqrtf(k));
}

float lenghtXY(Vector3& v)
{
    return std::sqrt(v.x * v.x + v.y * v.y);
}

Intersection CalcFlat(Ray r, LensInterface F)
{
    Intersection i;
    i.position = r.position + r.direction * ((F.center.z - r.position.z) / r.direction.z);
    i.norm = r.direction.z > 0 ? Vector3(0, 0, -1) : Vector3(0, 0, 1);
    i.theta = 0;
    i.hit = true;
    i.inverted = false;
    return i;
}

Intersection CalcSphere(Ray r, LensInterface F)
{
    Intersection i;
    Vector3 D = r.position - F.center;
    float B = D.Dot(r.direction);
    float C = D.Dot(D) - F.radius * F.radius;
    float B2_C = B * B - C;

    if (B2_C < 0)
    {
        i.hit = false;
        return i;
    }

    float sgn = (F.radius * r.direction.z) > 0 ? 1.f : -1.f;
    float t = std::sqrtf(B2_C) * sgn - B;
    i.position = r.position + r.direction * t;
    Vector3 _norm = i.position - F.center;
    _norm.Normalize();
    i.norm = _norm;
    i.theta = std::acos((-r.direction).Dot(i.norm));
    i.hit = true;
    i.inverted = t < 0;

    return i;
}

Matrix CalcRefractionMatrix(float r, float n1, float n2, bool f)
{
    Matrix m;
    if (!f)
    {
        m.m[1][0] = (n1 - n2) / (n2 * r);
        m.m[1][1] = n1 / n2;
    }
    else
    {
        m.m[1][1] = n1 / n2;
    }

    return m;
}

Matrix CalcTranslationMatrix(float d, float n)
{
    Matrix m;
    m.m[0][1] = d;

    return m;
}

Matrix CalcReflectionMatrix(float r, bool f)
{
    Matrix m;
    if (!f)
    {
        m.m[1][0] = 2.f / r;
    }

    return m;
}

void ParseLensComponents(LensDescription& lens)
{
    // Parse the lens components into the LensInterface
    lens.lensInterface.resize(lens.numOfLensComponents);
    for (int i = lens.numOfLensComponents - 1; i >= 0; --i)
    {
        LensParameters& entry = lens.lensComponents[i];
        lens.totalLensDistance += entry.d;

        float leftIor = i == 0 ? 1.f : lens.lensComponents[i - 1].n; // 왼쪽면 굴절률
        float rightIor = entry.n;                                    // 오른쪽면 굴절률

        if (rightIor != 1.f)
        {
            lens.minIor = std::min(lens.minIor, rightIor);
            lens.maxIor = std::max(lens.maxIor, rightIor);
        }

        Vector3 center = {0.f, 0.f, lens.totalLensDistance - entry.r};
        Vector3 n = {leftIor, 1.f, rightIor};

        LensInterface component = {center, entry.r, n, entry.h, entry.c, (float)entry.f, lens.totalLensDistance, entry.w};
    }

    int bounce1 = 2;
    int bounce2 = 1;
    int ghostIdx = 0;
    lens.ghosts.resize(lens.numOfGhosts);
    while (true)
    {
        if (bounce1 >= (int)(lens.lensInterface.size() - 1))
        {
            bounce2++;
            bounce1 = bounce2 + 1;
        }

        if (bounce2 >= (int)(lens.lensInterface.size() - 1))
        {
            break;
        }

        lens.ghosts[ghostIdx] = Vector4((float)bounce1, (float)bounce2, 0, 0);
        bounce1++;
        ghostIdx++;
    }
}

} // namespace OpticalSystem
} // namespace Riley