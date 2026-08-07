// The MIT License
// Copyright © 2021 Inigo Quilez
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions: The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software. THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


// Closest point on a sphere. For closest points on other primitives, check
//
//    https://www.shadertoy.com/playlist/wXsSzB



vec3 closestPointToSphere( vec3 p, vec3 cen, float rad )
{
    return cen + rad*normalize(p-cen);
}

//------------------------------------------------------------

// https://iquilezles.org/articles/distfunctions
float sdSphere( vec3 p, vec3 cen, float rad )
{
    return length(p-cen)-rad;
}

// https://iquilezles.org/articles/distfunctions
float sdCapsule( vec3 p, vec3 a, vec3 b, float r )
{
	vec3 pa = p-a, ba = b-a;
	float h = clamp( dot(pa,ba)/dot(ba,ba), 0.0, 1.0 );
	return length( pa - ba*h ) - r;
}

//------------------------------------------------------------

const float rad = 0.9;

vec2 map( in vec3 pos, bool showSphere, in vec3 samplePoint )
{

    // compute closest point to gPoint on the surace of the sphere
    vec3 closestPoint = closestPointToSphere(samplePoint, vec3(0.0), rad );
    
    // point
    vec2 res = vec2( sdSphere( pos, samplePoint, 0.06 ), 1.0 );
    
    // closest point
    {
    float d = sdSphere( pos, closestPoint, 0.06 );
    if( d<res.x ) res = vec2( d, 4.0 );
    }
    
    if( showSphere )
    {
    float d = sdSphere( pos, vec3(0.0), rad );
    if( d<res.x ) res =  vec2( d, 5.0 );
    }

    // segment
    {
    float d = sdCapsule( pos, samplePoint, closestPoint, 0.015 );
    if( d<res.x ) res =  vec2( d, 4.0 );
    }
    
    return res;
}

// https://iquilezles.org/articles/normalsSDF
vec3 calcNormal( in vec3 pos, in bool showSphere, in vec3 samplePoint )
{
    vec2 e = vec2(1.0,-1.0)*0.5773;
    const float eps = 0.0005;
    return normalize( e.xyy*map( pos + e.xyy*eps, showSphere, samplePoint ).x + 
					  e.yyx*map( pos + e.yyx*eps, showSphere, samplePoint ).x + 
					  e.yxy*map( pos + e.yxy*eps, showSphere, samplePoint ).x + 
					  e.xxx*map( pos + e.xxx*eps, showSphere, samplePoint ).x );
}

// https://iquilezles.org/articles/rmshadows
float calcSoftShadow(vec3 ro, vec3 rd, bool showSphere, in vec3 samplePoint )
{
    float res = 1.0;
    const float tmax = 2.0;
    float t = 0.001;
    for( int i=0; i<64; i++ )
    {
     	float h = map(ro + t*rd, showSphere, samplePoint).x;
        res = min( res, 64.0*h/t );
    	t += clamp(h, 0.01,0.5);
        if( res<-1.0 || t>tmax ) break;
        
    }
    res = max(res,-1.0);
    return 0.25*(1.0+res)*(1.0+res)*(2.0-res); // smoothstep, in [-1,1]
}

#if HW_PERFORMANCE==0
#define AA 1
#else
#define AA 2
#endif

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec3 tot = vec3(0.0);
    #if AA>1
    for( int m=0; m<AA; m++ )
    for( int n=0; n<AA; n++ )
    {
        // pixel coordinates
        vec2 o = vec2(float(m),float(n)) / float(AA) - 0.5;
        vec2 p = (2.0*(fragCoord+o)-iResolution.xy)/iResolution.y;
        // pixel sample
        ivec2 samp = ivec2(fragCoord)*AA + ivec2(m,n);
        // time sample
        float td = 0.5+0.5*sin(fragCoord.x*114.0)*sin(fragCoord.y*211.1);
        float time = iTime - (1.0/60.0)*(td+float(m*AA+n))/float(AA*AA-1);
        #else    
        // pixel coordinates
        vec2 p = (2.0*fragCoord-iResolution.xy)/iResolution.y;
        // pixel sample
        ivec2 samp = ivec2(fragCoord);
        // time sample
        float time = iTime;
        #endif

        // make sphere transparent
        bool showSphere = (texelFetch(iChannel0,samp&7,0).x>0.25);

        // animate camera
        float an = 0.25*time + 6.283185*iMouse.x/iResolution.x;
        vec3 ro = vec3( 2.4*cos(an), 0.7, 2.4*sin(an) );
        vec3 ta = vec3( 0.0, 0.0, 0.0 );

        // camera matrix
        vec3 ww = normalize( ta - ro );
        vec3 uu = normalize( cross(ww,vec3(0.2,1.0,0.0) ) );
        vec3 vv = normalize( cross(uu,ww));

        // animate point
        vec3 samplePoint = sin(time*0.8*vec3(1.0,1.1,1.2)+vec3(0.0,1.0,2.0));

	    // create view ray
        vec3 rd = normalize( p.x*uu + p.y*vv + 1.5*ww );

        // raycast
        const float tmax = 5.0;
        float t = 0.0;
        float m = -1.0;
        for( int i=0; i<256; i++ )
        {
            vec3 pos = ro + t*rd;
            vec2 hm = map(pos,showSphere,samplePoint);
            m = hm.y;
            if( hm.x<0.0001 || t>tmax ) break;
            t += hm.x;
        }
    
        // shade background
        vec3 col = vec3(0.05)*(1.0-0.2*length(p));
            
        // shade objects
        if( t<tmax )
        {
            // geometry
            vec3  pos = ro + t*rd;
            vec3  nor = calcNormal(pos,showSphere,samplePoint);

            // color
            vec3  mate = 0.55 + 0.45*cos( m + vec3(0.0,1.0,1.5) );
            // show distance isolines
            if( abs(m-5.0)<0.5 )
            {
                float dref = sdSphere( samplePoint, vec3(0.0), rad );
                float dsam = length(pos-samplePoint);
                mate += 0.25*smoothstep(0.8,0.9,sin((dsam-dref)*100.0))*exp2(-12.0*(dsam-dref)*(dsam-dref));
            }
            
            // lighting	
            col = vec3(0.0);
            {
              // key light
              vec3  lig = normalize(vec3(0.3,0.7,0.2));
              float dif = clamp( dot(nor,lig), 0.0, 1.0 );
              if( dif>0.001 ) dif *= calcSoftShadow(pos+nor*0.001,lig,showSphere,samplePoint);
              col += mate*vec3(1.0,0.9,0.8)*dif;
            }
            {
              // dome light
              float dif = 0.5 + 0.5*nor.y;
              col += mate*vec3(0.2,0.3,0.4)*dif;
            }
        }

        // gamma        
        col = pow( col, vec3(0.4545) );
	    tot += col;
    #if AA>1
    }
    tot /= float(AA*AA);
    #endif

    // cheap dithering
    tot += sin(fragCoord.x*114.0)*sin(fragCoord.y*211.1)/512.0;

	fragColor = vec4( tot, 1.0 );
}