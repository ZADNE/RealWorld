const vec4 CALM  = vec4(0.2, 0.5, 1.0, 1.0);
const vec4 STREAM  = vec4(0.6, 0.7, 1.0, 0.8);

const float MAX = 16.0;

void mainImage(out vec4 fragColor, in vec2 fragCoord ){
    float a = fragCoord.x/iResolution.x * MAX - MAX * 0.5;
    vec2 speed;
    speed.x = abs(0.5 - fract(a));
    speed.y = abs(trunc(a) / MAX * 2.0);

    fragColor = mix(CALM, STREAM, clamp(speed.x + speed.y, 0.0, 1.0));
    
    //fragColor = CALM;
    //fragColor = STREAM;
}