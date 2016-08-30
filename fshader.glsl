#version 150

//gouraud
in  vec4 color;
//phong, cel
in vec3 N;
in vec3 pos;
in vec4 Color;

out vec4 fColor;

uniform int mode;
uniform vec4 AmbientProduct, DiffuseProduct, SpecularProduct;
uniform mat4 ModelView;
uniform vec4 LightPosition;
uniform float Shininess;
//phong, cel
uniform vec4 idColor;
uniform int colorFlag;
//cel
uniform int Shades;
uniform int outlineMode;

void drawOutline(){
    fColor = vec4(0.0, 0.0, 0.0, 1.0);     
}

void cel(){
     if(outlineMode == 1){
        drawOutline();
     } else {
     int i;
     vec4 temp_diffuse;
     float fraction, diffuse_sum;
     diffuse_sum = DiffuseProduct.r + DiffuseProduct.g + DiffuseProduct.b;

     vec4 ambient, diffuse, specular;
     vec3 NN = normalize(N);
     vec3 E = normalize(-pos);
     vec3 L = normalize((LightPosition).xyz - pos);

     ambient = AmbientProduct;

     float dr = max(dot(L, NN), 0.0);
     diffuse = dr * DiffuseProduct;
     for(i = 0; i < Shades; i++){
           if(diffuse.r + diffuse.g + diffuse.b >= diffuse_sum * (1.0 * i / Shades) - 0.4){
           fraction = (1.0 * i / Shades) + 0.05;
           temp_diffuse.r = DiffuseProduct.r * fraction;
           temp_diffuse.g = DiffuseProduct.g * fraction;
           temp_diffuse.b = DiffuseProduct.b * fraction;
           }
     }
     diffuse.r = temp_diffuse.r;
     diffuse.g = temp_diffuse.g;
     diffuse.b = temp_diffuse.b;
     
     if(idColor.r + idColor.g + idColor.b > 0.0){
         fColor = vec4(idColor.r, idColor.g, idColor.b, 1.0);
     } else if(colorFlag == 1){
         fColor = vec4(Color.r, Color.g, Color.b, 1.0);
     } else {
              fColor = vec4((diffuse + ambient).xyz, 1.0);
     }
}
}

void phong(){
     vec4 ambient, diffuse, specular;
     vec3 NN = normalize(N);
     vec3 E = normalize(-pos);
     vec3 L = normalize((LightPosition).xyz - pos);
     vec3 R = normalize(-reflect(L, NN));

     ambient = AmbientProduct;

     float dr = max(dot(L, NN), 0.0);
     diffuse = dr * DiffuseProduct;

     float sr = pow(max(dot(E, R), 0.0), Shininess);
     specular = sr * SpecularProduct;
     specular = clamp(specular, 0.0, 1.0);

     if(idColor.r + idColor.g + idColor.b > 0.0){
          fColor = vec4(idColor.r, idColor.g, idColor.b, 1.0);
     } else if(colorFlag == 1){
          fColor = vec4(Color.r, Color.g, Color.b, 1.0);
     } else {
          fColor = vec4((ambient + diffuse + specular).xyz, 1.0);
     }
}

void gouraud(){
    fColor = color;     
}

void main()
{
    if(mode == 0){
       gouraud();
    } else if(mode == 1){
       phong();
    } else if(mode == 2){
      cel();
    }


}
