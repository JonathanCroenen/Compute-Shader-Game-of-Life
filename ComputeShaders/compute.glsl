#version 460 core
layout (local_size_x = 1, local_size_y = 1) in;
layout (binding = 0, rgba32f) uniform image2D image0;
layout (binding = 1, rgba32f) uniform image2D image1;

uniform int currentBuffer;
uniform bool pause;
uniform vec2 mousePos;

void SetPixel();
void CalcColor(layout (rgba32f) image2D readImage,layout (rgba32f) image2D writeImage);

void main()
{
	if (!pause){
		if (currentBuffer == 0){
			CalcColor(image0, image1);
		}
		else if (currentBuffer == 1){
			CalcColor(image1, image0);
		}
	} else if (pause && (mousePos != vec2(-1.0, -1.0))){
		SetPixel();
	}
}

void SetPixel()
{
	ivec2 mousePosi = ivec2(mousePos);
	if (ivec2(gl_GlobalInvocationID.xy) == mousePosi){
		imageStore(image0, mousePosi, vec4(0.0, 0.0, 0.0, 1.0));
		imageStore(image1, mousePosi, vec4(0.0, 0.0, 0.0, 1.0));
	}
	//imageStore(image0, ivec2(gl_GlobalInvocationID.xy), vec4(mousePos/imageSize(image0), 0.0, 1.0));
	//imageStore(image1, ivec2(gl_GlobalInvocationID.xy), vec4(mousePos/imageSize(image0), 0.0, 1.0));
}


void CalcColor(layout (rgba32f) image2D readImage, layout (rgba32f) image2D writeImage)
{
	ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);
	ivec2 dims = imageSize(readImage);

	vec4 self = imageLoad(readImage, ivec2(pixelCoords.x, pixelCoords.y));
	bool alive = true ? (self == vec4(0.0, 0.0, 0.0, 1.0)) : false;

	vec4 neighbours[8];
	if (pixelCoords.x == 0)
		neighbours[0] = vec4(1.0);
	else
		neighbours[0] = imageLoad(readImage, ivec2(pixelCoords.x-1, pixelCoords.y));
	if (pixelCoords.x == dims.x)
		neighbours[1] = vec4(1.0);
	else
		neighbours[1] = imageLoad(readImage, ivec2(pixelCoords.x+1, pixelCoords.y));
	if (pixelCoords.y == 0)
		neighbours[2] = vec4(1.0);
	else
		neighbours[2] = imageLoad(readImage, ivec2(pixelCoords.x, pixelCoords.y-1));
	if (pixelCoords.y == dims.y)
		neighbours[3] = vec4(1.0);
	else
		neighbours[3] = imageLoad(readImage, ivec2(pixelCoords.x, pixelCoords.y+1));
	
	if (pixelCoords.x == 0 || pixelCoords.y == 0)
		neighbours[4] = vec4(1.0);
	else
		neighbours[4] = imageLoad(readImage, ivec2(pixelCoords.x-1, pixelCoords.y-1));
	if (pixelCoords.x == dims.x || pixelCoords.y == 0)
		neighbours[5] = vec4(1.0);
	else
		neighbours[5] = imageLoad(readImage, ivec2(pixelCoords.x+1, pixelCoords.y-1));
	if (pixelCoords.x == dims.x || pixelCoords.y == dims.y)
		neighbours[6] = vec4(1.0);
	else
		neighbours[6] = imageLoad(readImage, ivec2(pixelCoords.x+1, pixelCoords.y+1));
	if (pixelCoords.x == 0 || pixelCoords.y == dims.y)
		neighbours[7] = vec4(1.0);
	else
		neighbours[7] = imageLoad(readImage, ivec2(pixelCoords.x-1, pixelCoords.y+1));

	int count = 0;
	for (int i = 0; i < 8; i++){
		if (neighbours[i] == vec4(0.0, 0.0, 0.0, 1.0)){
			count++;
		}
	}

	vec4 color;
	if (alive && count < 2)
		color = vec4(1.0);
	else if (alive && (count == 2 || count == 3))
		color = vec4(0.0, 0.0, 0.0, 1.0);
	else if (alive && count > 3)
		color = vec4(1.0);
	else if (!alive && count == 3)
		color = vec4(0.0, 0.0, 0.0, 1.0);
	else
		color = vec4(1.0);

	//if (alive)
		//color = vec4(0.0, 0.0, 0.0, 1.0);
	//color = imageLoad(writeImage, ivec2(pixelCoords.x, pixelCoords.y));
	
	imageStore(writeImage, pixelCoords, color);
}