#include "vcl/vcl.hpp"
#include "vegetation.hpp"

using namespace vcl;

static vec3 coordonnee(float s, vec3 point0, vec3 point1, vec3 point2, vec3 point3) {
	vec3 d1 = 2.f * vcl::normalize(point2 - point0);
	vec3 d2 = 2.f * vcl::normalize(point3 - point1);
	float s2 = s * s;
	float s3 = s2 * s;
	return (2 * s3 - 3 * s2 + 1) * point1 + (s3 - 2 * s2 + s) * d1
		+ (-2 * s3 + 3 * s2) * point2 + (s3 - s2) * d2;
}

static buffer<vec3> spine(int pointsPerSpine, vec3 p0, vec3 p1, vec3 p2, vec3 p3) {
	buffer<vec3> Spine;
	for (int compteur = 0; compteur < pointsPerSpine + 1; compteur++) {
		Spine.push_back(coordonnee((float)compteur / pointsPerSpine, p0, p1, p2, p3));
	}
	return Spine;
}

static buffer<vec3> circle(vec3 p0, vec3 p, vec3 d, float r, int n) {
	buffer<vec3> Circle;
	//vec3 vectorPipeau = p - p0 + vcl::vec3({.001f, .002f, .003f});
	vec3 vectorPipeau = vcl::vec3({ .8647f, .5897f, .5292f });
	vec3 d1 = vcl::normalize(vcl::cross(d, vectorPipeau));
	vec3 d2 = vcl::normalize(vcl::cross(d, d1));
	float dTheta = 2 * pi / n;
	float theta = 0;
	for (int compteur = 0; compteur < n; compteur++) {
		Circle.push_back(p + cos(theta) * r * d1 + sin(theta) * r * d2);
		theta += dTheta;
	}
	return Circle;
}

static vec3 point0 = { 0,-.3f,-3.0f };
static vec3 point1 = { 0,0,0 };
static vec3 point2 = { 0,-.3f,3.f };
static vec3 point3 = { 0,-1.f,6.f };
static vec3 point4 = { 0,-2.f,8.5f };
static vec3 point5 = { 0,-3.f,11.f };
static vec3 point6 = { 0,-3.2f,14.f };
static vec3 point7 = { 0,-2.5f,17.f };
static vec3 point8 = { 0,0,19.f };
static vec3 point9 = { 0,3.f,18.5f };
static vec3 point10 = { 0,5.f,16.f };
static vec3 point11 = { 0,4.5f,14.f };
static vec3 point12 = { 0,3.f,12.5f };
static vec3 point13 = { 0,1.f,13.f };
static vec3 point14 = { 0,.3f,13.5f };
static vec3 point15 = { 0,0,14.f };

void createPlant(hierarchy_mesh_drawable& hierarchy) {
	int nombreTroncons = 13;
	buffer<vec3> pointsInterpolation;
	pointsInterpolation.push_back(point0);
	pointsInterpolation.push_back(point1);
	pointsInterpolation.push_back(point2);
	pointsInterpolation.push_back(point3);
	pointsInterpolation.push_back(point4);
	pointsInterpolation.push_back(point5);
	pointsInterpolation.push_back(point6);
	pointsInterpolation.push_back(point7);
	pointsInterpolation.push_back(point8);
	pointsInterpolation.push_back(point9);
	pointsInterpolation.push_back(point10);
	pointsInterpolation.push_back(point11);
	pointsInterpolation.push_back(point12);
	pointsInterpolation.push_back(point13);
	pointsInterpolation.push_back(point14);
	pointsInterpolation.push_back(point15);
	buffer<int> pointsPerSpine;
	pointsPerSpine.push_back(100); //p1-p2
	pointsPerSpine.push_back(100); //p2-p3
	pointsPerSpine.push_back(100); //p3-p4
	pointsPerSpine.push_back(100); //p4-p5
	pointsPerSpine.push_back(100); //p5-p6
	pointsPerSpine.push_back(100); //p6-p7
	pointsPerSpine.push_back(100); //p7-p8
	pointsPerSpine.push_back(100); //p8-p9
	pointsPerSpine.push_back(100); //p9-p10
	pointsPerSpine.push_back(100); //p10-p11
	pointsPerSpine.push_back(100); //p11-p12
	pointsPerSpine.push_back(100); //p12-p13
	pointsPerSpine.push_back(100); //p13-p14
	buffer<float> rayonsInterpolation;
	rayonsInterpolation.push_back(.65f); //p1
	rayonsInterpolation.push_back(.6f); //p2
	rayonsInterpolation.push_back(.55f); //p3
	rayonsInterpolation.push_back(.5); //p4
	rayonsInterpolation.push_back(.45f); //p5
	rayonsInterpolation.push_back(.4f); //p6
	rayonsInterpolation.push_back(.35f); //p7
	rayonsInterpolation.push_back(.3f); //p8
	rayonsInterpolation.push_back(.25f); //p9
	rayonsInterpolation.push_back(.2f); //p10
	rayonsInterpolation.push_back(.15f); //p11
	rayonsInterpolation.push_back(.1f); //p12
	rayonsInterpolation.push_back(.05f); //p13
	rayonsInterpolation.push_back(.01f); //p14
	buffer<int> numberPointsCircle;
	numberPointsCircle.push_back(100); //p1
	numberPointsCircle.push_back(100); //p2
	numberPointsCircle.push_back(100); //p3
	numberPointsCircle.push_back(100); //p4
	numberPointsCircle.push_back(100); //p5
	numberPointsCircle.push_back(100); //p6
	numberPointsCircle.push_back(100); //p7
	numberPointsCircle.push_back(100); //p8
	numberPointsCircle.push_back(100); //p9
	numberPointsCircle.push_back(100); //p10
	numberPointsCircle.push_back(100); //p11
	numberPointsCircle.push_back(100); //p12
	numberPointsCircle.push_back(100); //p13
	numberPointsCircle.push_back(100); //p14
	vec3 colorLow = { .38f,.33f,.24f };
	vec3 colorHigh = { .38f,.88f,.69f };
	buffer<mesh_drawable> troncons;

	buffer<buffer<vec3>> spines; // La spine d'indice 0 est spine1
	for (int indiceTroncon = 0; indiceTroncon < nombreTroncons; indiceTroncon++) {
		spines.push_back(spine(pointsPerSpine[indiceTroncon], pointsInterpolation[indiceTroncon],
			pointsInterpolation[indiceTroncon + 1], pointsInterpolation[indiceTroncon + 2], pointsInterpolation[indiceTroncon + 3]));
		buffer<vec3> directionsLocales;
		directionsLocales.push_back(vcl::normalize(spines[indiceTroncon][1] - pointsInterpolation[indiceTroncon + 1]));
		for (int compteur = 1; compteur < pointsPerSpine[indiceTroncon] - 1; compteur++) {
			directionsLocales.push_back(vcl::normalize(spines[indiceTroncon][compteur + 1] - spines[indiceTroncon][compteur - 1]));
		}
		directionsLocales.push_back(vcl::normalize(pointsInterpolation[indiceTroncon + 2]
			- spines[indiceTroncon][pointsPerSpine[indiceTroncon] - 1]));
		buffer<vec3> pointsLocaux;
		pointsLocaux.push_back(circle(pointsInterpolation[indiceTroncon], spines[indiceTroncon][0],
			directionsLocales[0], rayonsInterpolation[indiceTroncon], numberPointsCircle[indiceTroncon])
			- pointsInterpolation[indiceTroncon + 1]);
		for (int compteur = 1; compteur < pointsPerSpine[indiceTroncon]; compteur++) {
			float rayonLocal = rayonsInterpolation[indiceTroncon]
				+ (rayonsInterpolation[indiceTroncon + 1] - rayonsInterpolation[indiceTroncon])
				* (float)compteur / pointsPerSpine[indiceTroncon];
			pointsLocaux.push_back(circle(spines[indiceTroncon][compteur - 1], spines[indiceTroncon][compteur], directionsLocales[compteur],
				rayonLocal, numberPointsCircle[indiceTroncon]) - pointsInterpolation[indiceTroncon + 1]);
		}
		pointsLocaux.push_back(circle(pointsInterpolation[indiceTroncon + 1], spines[indiceTroncon][pointsPerSpine[indiceTroncon]],
			directionsLocales[pointsPerSpine[indiceTroncon] - 1], rayonsInterpolation[indiceTroncon + 1], numberPointsCircle[indiceTroncon + 1])
			- pointsInterpolation[indiceTroncon + 1]);
		mesh meshTroncon;
		meshTroncon.position = pointsLocaux;
		for (int indiceVertebra = 0; indiceVertebra < pointsPerSpine[indiceTroncon]; indiceVertebra++) {
			int base = indiceVertebra * numberPointsCircle[indiceTroncon];
			for (int indicePointOnCircle = 0; indicePointOnCircle < numberPointsCircle[indiceTroncon] - 1;
				indicePointOnCircle++) {
				meshTroncon.connectivity.push_back({ base + indicePointOnCircle,
					base + indicePointOnCircle + 1,
					base + numberPointsCircle[indiceTroncon] + indicePointOnCircle });
				meshTroncon.connectivity.push_back({ base + indicePointOnCircle + 1,
					base + numberPointsCircle[indiceTroncon] + indicePointOnCircle + 1,
					base + numberPointsCircle[indiceTroncon] + indicePointOnCircle });
			}
			meshTroncon.connectivity.push_back({ base + numberPointsCircle[indiceTroncon] - 1,
				base, base + 2 * numberPointsCircle[indiceTroncon] - 1 });
			meshTroncon.connectivity.push_back({ base, base + numberPointsCircle[indiceTroncon],
				base + 2 * numberPointsCircle[indiceTroncon] - 1 });
		}
		meshTroncon.fill_empty_field();
		mesh_drawable troncon = mesh_drawable(meshTroncon);
		troncon.shading.phong.specular = 0.0f;
		troncon.shading.color = colorLow + (float)indiceTroncon / nombreTroncons * (colorHigh - colorLow);
		troncons.push_back(troncon);
	}

	mesh_drawable disque = mesh_drawable(mesh_primitive_disc(1.f, { 0,0,0 }, { 0,0,1.f }, 40));
	hierarchy.add(troncons[0], "troncon " + std::to_string(0));
	for (int compteur = 1; compteur < nombreTroncons; compteur++) {
		hierarchy.add(troncons[compteur], "troncon " + std::to_string(compteur), "troncon " + std::to_string(compteur - 1),
			pointsInterpolation[compteur + 1] - pointsInterpolation[compteur]);
	}
}

void plantAnimation(hierarchy_mesh_drawable& hierarchy, float t) {
	hierarchy["troncon 1"].transform.rotate = rotation({ 1.f,0,0 }, .05f * std::sin(2 * 3.14f * (t)));
	hierarchy["troncon 2"].transform.rotate = rotation({ 1.f,0,0 }, .05f * std::sin(2 * 3.14f * (t - .3f)));
	hierarchy["troncon 3"].transform.rotate = rotation({ 1.f,0,0 }, .05f * std::sin(2 * 3.14f * (t - .4f)));
	hierarchy["troncon 4"].transform.rotate = rotation({ 1.f,0,0 }, .05f * std::sin(2 * 3.14f * (t - .5f)));
	hierarchy["troncon 5"].transform.rotate = rotation(vcl::normalize(point6 - point4), .1f * std::sin(2 * 3.14f * (t - .6f)));
	hierarchy["troncon 6"].transform.rotate = rotation(vcl::normalize(point7 - point5), .1f * std::sin(2 * 3.14f * (t - .9f)));
	hierarchy["troncon 7"].transform.rotate = rotation(vcl::normalize(point8 - point6), .1f * std::sin(2 * 3.14f * (t - 1.2f)));
	hierarchy["troncon 8"].transform.rotate = rotation(vcl::normalize(point9 - point7), .1f * std::sin(2 * 3.14f * (t - 1.3f)));
	hierarchy["troncon 9"].transform.rotate = rotation(vcl::normalize(point10 - point8), .1f * std::sin(2 * 3.14f * (t - 1.4f)));
	hierarchy["troncon 10"].transform.rotate = rotation(vcl::normalize(point11 - point9), .1f * std::sin(2 * 3.14f * (t - 1.5f)));

	hierarchy.update_local_to_global_coordinates();
}

buffer<buffer<float>> plantSpawn(int nombrePousses, vec3 colorLow, vec3 colorHigh, float sizeMax) {
	buffer<buffer<float>> infos;
	buffer<float> thetaList; //[0]
	buffer<float> phiList; //[1]
	buffer<float> xList; //[2]
	buffer<float> yList; //[3]
	buffer<float> zList; //[4]
	buffer<float> alphaList; //[5]
	buffer<float> redList; //[6]
	buffer<float> greenList; //[7]
	buffer<float> blueList; //[8]
	buffer<float> sizeList; //[9]
	for (int compteur = 0; compteur < nombrePousses; compteur++) {
		float theta = (float)rand() / RAND_MAX * pi;
		float phi = (float)rand() / RAND_MAX * 2 * pi;
		thetaList.push_back(theta);
		phiList.push_back(phi);
		xList.push_back(cos(theta));
		yList.push_back(sin(theta) * cos(phi));
		zList.push_back(sin(theta) * sin(phi));
		alphaList.push_back((float)rand() / RAND_MAX * 2 * pi);
		redList.push_back(colorLow.x + (float)rand() / RAND_MAX * (colorHigh.x - colorLow.x));
		greenList.push_back(colorLow.y + (float)rand() / RAND_MAX * (colorHigh.y - colorLow.y));
		blueList.push_back(colorLow.z + (float)rand() / RAND_MAX * (colorHigh.z - colorLow.z));
		sizeList.push_back(sizeMax * 1.0f / 5 + (float)rand() / RAND_MAX * sizeMax * 4.0f / 3);
	}
	infos.push_back(thetaList);
	infos.push_back(phiList);
	infos.push_back(xList);
	infos.push_back(yList);
	infos.push_back(zList);
	infos.push_back(alphaList);
	infos.push_back(redList);
	infos.push_back(blueList);
	infos.push_back(greenList);
	infos.push_back(sizeList);
	return infos;
}