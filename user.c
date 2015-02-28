# include <stdio.h>
# include <stdlib.h>



float verticesX[9000][2];
float verticesY[9000][2];
float colors[9000][3];



struct Coord
{
	float x, y;
};

struct Coord mid(struct Coord a, struct Coord b)
{
	struct Coord c;
	c.x = (a.x + b.x) / 2;
	c.y = (a.y + b.y) / 2;
	return c;
}

float dist(struct Coord a, struct Coord b)
{
	return sqrt(pow(b.x - a.x, 2) + pow(b.y - a.y, 2));
}


float scale(const float valueIn, const float baseMin, const float baseMax, const float limitMin, const float limitMax) {
	return ((limitMax - limitMin) * (valueIn - baseMin) / (baseMax - baseMin)) + limitMin;
}


unsigned int rand_interval(unsigned int min, unsigned int max)
{
	return (rand() % (max + 1 - min)) + min;
}
int index = 0;
void drawSierpinski(int iter, struct Coord a, struct Coord b, struct Coord c)
{
	
	if (iter >= 8)  // if lines are only 2 pixels in length
	{
		return;
	}
	else
	{
		drawSierpinski(iter + 1, a, mid(a, b), mid(a, c));
		verticesX[index][0] = a.x;
		verticesX[index][1] = mid(a, b).x;
		verticesX[index][2] = mid(a, c).x;
		verticesY[index][0] = a.y;
		verticesY[index][1] = mid(a, b).y;
		verticesY[index][2] = mid(a, c).y;
		colors[index][0] = scale(rand_interval(0, 255),0,255,0,1);
		colors[index][1] = scale(rand_interval(0, 255), 0, 255, 0, 1);
		colors[index][2] = scale(rand_interval(0, 255), 0, 255, 0, 1);
		drawSierpinski(iter + 1, mid(a, b), b, mid(b, c));
		index++;
		verticesX[index][0] = mid(a, b).x;
		verticesX[index][1] = b.x;
		verticesX[index][2] = mid(b, c).x;
		verticesY[index][0] = mid(a, b).y;
		verticesY[index][1] = b.y;
		verticesY[index][2] = mid(b, c).y;
		colors[index][0] = scale(rand_interval(0, 255), 0, 255, 0, 1);
		colors[index][1] = scale(rand_interval(0, 255), 0, 255, 0, 1);
		colors[index][2] = scale(rand_interval(0, 255), 0, 255, 0, 1);
		drawSierpinski(iter + 1, mid(a, c), mid(b, c), c);
		index++;
		verticesX[index][0] = mid(a, c).x;
		verticesX[index][1] = mid(b, c).x;
		verticesX[index][2] = c.x;
		verticesY[index][0] = mid(a, c).y;
		verticesY[index][1] = mid(b, c).y;
		verticesY[index][2] = c.y;
		colors[index][0] = scale(rand_interval(0, 255), 0, 255, 0, 1);
		colors[index][1] = scale(rand_interval(0, 255), 0, 255, 0, 1);
		colors[index][2] = scale(rand_interval(0, 255), 0, 255, 0, 1);
	}

}

	int main()
	{
		struct Coord top, left, right;
		left.x = -1;
		left.y = 1;
		top.x = 0;
		top.y = -1;
		right.x = 1;
		right.y = 1;
		
		drawSierpinski(0, left, top, right);
		printf("index = %d, %d\n", index, rand_interval(0,255));
		int tri = 0;
		for (int i = 0; i < 120; i++){
			for (int j = 0; j < 2; j++){
				printf("(%f,%f,%d) ", verticesX[i][j], verticesY[i][j], rand_interval(0, 255));
			}
			printf("\nColor = (%f,%f,%f)", colors[i][0], colors[i][1], colors[i][2]);
			
			
			getch();
			printf("\n");
		}
		
		getch();
	}


