
#include "circa.h"

using namespace circa;

// Decide if point Px/Py is inside triangle defined by
// (Ax,Ay) (Bx,By) (Cx,Cy)
static bool InsideTriangle(float Ax, float Ay,
                  float Bx, float By,
                  float Cx, float Cy,
                  float Px, float Py);

// Compute area of a contour/polygon
static float Area(List* contour);

// Triangulate a contour/polygon, places results in STL vector
// as series of triangles.
static bool Process(List* contour, List* result);

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static const float EPSILON=0.0000000001f;

float Area(List* contour)
{
    int n = list_length(contour);

    float A = 0.0f;

    for (int p=n-1,q=0; q<n; p=q++)
    {
        float p_x, p_y, q_x, q_y;
        get_point(list_get(contour, p), &p_x, &p_y);
        get_point(list_get(contour, q), &q_x, &q_y);

        A += p_x * q_y - q_x * p_y;
    }
    return A * 0.5f;
}

bool InsideTriangle(float Ax, float Ay,
                      float Bx, float By,
                      float Cx, float Cy,
                      float Px, float Py)
{
    float ax, ay, bx, by, cx, cy, apx, apy, bpx, bpy, cpx, cpy;
    float cCROSSap, bCROSScp, aCROSSbp;

    ax = Cx - Bx;    ay = Cy - By;
    bx = Ax - Cx;    by = Ay - Cy;
    cx = Bx - Ax;    cy = By - Ay;
    apx= Px - Ax;    apy= Py - Ay;
    bpx= Px - Bx;    bpy= Py - By;
    cpx= Px - Cx;    cpy= Py - Cy;

    aCROSSbp = ax*bpy - ay*bpx;
    cCROSSap = cx*apy - cy*apx;
    bCROSScp = bx*cpy - by*cpx;

    return ((aCROSSbp >= 0.0f) && (bCROSScp >= 0.0f) && (cCROSSap >= 0.0f));
};

bool Snip(List* contour, int u, int v, int w, int n, int *V)
{
    int p;
    float Ax, Ay, Bx, By, Cx, Cy, Px, Py;

    get_point(list_get(contour, V[u]), &Ax, &Ay);
    get_point(list_get(contour, V[v]), &Bx, &By);
    get_point(list_get(contour, V[w]), &Cx, &Cy);

    if ( EPSILON > (((Bx-Ax)*(Cy-Ay)) - ((By-Ay)*(Cx-Ax))) )
        return false;

    for (p=0; p<n; p++)
    {
        if ( (p == u) || (p == v) || (p == w) )
            continue;
        get_point(list_get(contour, V[p]), &Px, &Py);
        if (InsideTriangle(Ax,Ay,Bx,By,Cx,Cy,Px,Py))
            return false;
    }

    return true;
}

bool Process(List* contour, List* result)
{
    set_list(result, 0);

    // allocate and initialize list of Vertices in polygon

    int n = list_length(contour);
    if ( n < 3 )
        return false;

    int *V = new int[n];

    // we want a counter-clockwise polygon in V
    if ( 0.0f < Area(contour) )
        for (int v=0; v<n; v++) V[v] = v;
    else
        for (int v=0; v<n; v++) V[v] = (n-1)-v;

    int nv = n;

    /*    remove nv-2 Vertices, creating 1 triangle every time */
    int count = 2*nv;     /* error detection */

    for (int m=0, v=nv-1; nv>2; )
    {
        /* if we loop, it is probably a non-simple polygon */
        if (0 >= (count--))
        {
            //** Triangulate: ERROR - probable bad polygon!
            delete V;
            return false;
        }

        // three consecutive vertices in current polygon, <u,v,w>
        int u = v; if (nv <= u) u = 0;       // previous
        v = u+1; if (nv <= v) v = 0;         // new v
        int w = v+1; if (nv <= w) w = 0;     // next

        if ( Snip(contour,u,v,w,nv,V) )
        {
            int a,b,c,s,t;

            // true names of the vertices
            a = V[u]; b = V[v]; c = V[w];

            // write triangle to output
            copy(list_get(contour, a), list_append(result));
            copy(list_get(contour, b), list_append(result));
            copy(list_get(contour, c), list_append(result));

            m++;

            // remove v from remaining polygon
            for(s=v,t=v+1; t<nv; s++,t++)
                V[s] = V[t];
            nv--;

            // resest error detection counter
            count = 2*nv;
        }
    }

    delete V;

    return true;
}

extern "C" {

CA_FUNCTION(triangulate__process)
{
    bool result = Process((List*) INPUT(0), (List*) OUTPUT);
    if (!result)
        RAISE_ERROR("Bad input");
}

}
/*
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc,char **argv)
{
  // Small test application demonstrating the usage of the triangulate
  // class.

  // Create a pretty complicated little contour by pushing them onto
  // an stl vector.

  Vector2dVector a;

  a.push_back( Vector2d(0,6));
  a.push_back( Vector2d(0,0));
  a.push_back( Vector2d(3,0));
  a.push_back( Vector2d(4,1));
  a.push_back( Vector2d(6,1));
  a.push_back( Vector2d(8,0));
  a.push_back( Vector2d(12,0));
  a.push_back( Vector2d(13,2));
  a.push_back( Vector2d(8,2));
  a.push_back( Vector2d(8,4));
  a.push_back( Vector2d(11,4));
  a.push_back( Vector2d(11,6));
  a.push_back( Vector2d(6,6));
  a.push_back( Vector2d(4,3));
  a.push_back( Vector2d(2,6));

  // allocate an STL vector to hold the answer.

  Vector2dVector result;

  //  Invoke the triangulator to triangulate this polygon.
  Process(a,result);

  // print out the results.
  int tcount = result.size()/3;

  for (int i=0; i < tcount; i++)
  {
    const Vector2d &p1 = result[i*3+0];
    const Vector2d &p2 = result[i*3+1];
    const Vector2d &p3 = result[i*3+2];
    printf("Triangle %d => (%0.0f,%0.0f) (%0.0f,%0.0f) (%0.0f,%0.0f)\n",i+1,p1.GetX(),p1.GetY(),p2.GetX(),p2.GetY(),p3.GetX(),p3.GetY());
  }
  return 0;
}
*/
