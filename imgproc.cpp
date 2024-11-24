#include "framework.h"
#include "iRecHS2s.h"

#define ThresholdRange 40
#define PreShift 5
#define ReduceImageStep 3


//#define CONWIN 1


Cluster::Cluster()
{
	nlabel = 0;
	rsize = { 0,0,0 };
	imgsize = { 0,0 };
	labelPtr = NULL;
	labelimgPtr = NULL;
}
Cluster::~Cluster()
{
	if (labelPtr != NULL) {
		delete[] labelPtr;
	}
	if (labelimgPtr != NULL) {
		delete[] labelimgPtr;
	}
}
void Cluster::SetArray(int w, int h)
{
	if ((w != rsize.w) || (h != rsize.h)) {
		if (rsize.wh < w * h) {
			rsize.wh = w * h;
			if (labelPtr != NULL) {
				delete[] labelPtr;
			}
			if (labelimgPtr != NULL) {
				delete[] labelimgPtr;
			}
			labelPtr = new LABEL[rsize.wh]();
			labelimgPtr = new unsigned int[rsize.wh]();
		}
		rsize.w = static_cast<int>(w);
		rsize.h = static_cast<int>(h);
	}
	nlabel = 0;
}
int Cluster::JoinGroup(int id1, int id2)
{
	int live, death, i, j, k;

	if (labelPtr[id1].co > labelPtr[id2].co) {
		live = id1;
		death = id2;
	}
	else {
		live = id2;
		death = id1;
	}
	if (labelPtr[live].t > labelPtr[death].t) labelPtr[live].t = labelPtr[death].t;
	if (labelPtr[live].b < labelPtr[death].b) labelPtr[live].b = labelPtr[death].b;
	if (labelPtr[live].l > labelPtr[death].l) labelPtr[live].l = labelPtr[death].l;
	if (labelPtr[live].r < labelPtr[death].r) labelPtr[live].r = labelPtr[death].r;
	labelPtr[live].co += labelPtr[death].co;
	labelPtr[death].co = 0;

	for (j = labelPtr[death].t; j <= labelPtr[death].b; j++) {
		for (i = labelPtr[death].l; i <= labelPtr[death].r; i++) {
			k = j * rsize.w + i;
			if (labelimgPtr[k] == death) labelimgPtr[k] = live;
		}
	}
	return(live);
}
void Cluster::CheckPoint(int x, int y)
{
	int id[2], cid, i, tmp;

	int ct;

	ct = x + y * rsize.w;

	id[0] = (x != 0) ? labelimgPtr[ct - 1] : 0;
	id[1] = (y != 0) ? labelimgPtr[ct - rsize.w] : 0;

	for (i = cid = 0; i < 2; i++) {
		if (id[i] != 0) {
			if (cid != 0) {
				if (cid != id[i]) JoinGroup(id[i], cid);
			}
			else {
				cid = id[i];
				labelPtr[cid].co++;
				labelPtr[cid].b = ct / rsize.w;
				tmp = ct % rsize.w;
				if (labelPtr[cid].r < tmp) labelPtr[cid].r = tmp;
				labelimgPtr[ct] = cid;
			}
		}
	}
	if (cid == 0) {
		cid = ++nlabel;
		labelimgPtr[ct] = cid;
		labelPtr[cid].co = 1;
		labelPtr[cid].t = labelPtr[cid].b = ct / rsize.w;
		labelPtr[cid].l = labelPtr[cid].r = ct % rsize.w;
	}
}


PupilEdge::PupilEdge()
{
	mem = { 0,0 };
	cur = { 0,0 };
	Xe = NULL;
	Ye = NULL;
	x_num = y_num = 0;
}
PupilEdge::~PupilEdge()
{
	if (Xe != NULL) {
		delete[] Xe;
	}
	if (Ye != NULL) {
		delete[] Ye;
	}
}
void PupilEdge::Set(int w, int h)
{
	if (w != cur.w) {
		cur.w = w;
		if (w > mem.w) {
			if (Ye != NULL) {
				delete[] Ye;
			}
			Ye = new YEdge[w]();
			mem.w = w;
		}
	}
	if (h != cur.h) {
		cur.h = h;
		if (h > mem.h) {
			if (Xe != NULL) {
				delete[] Xe;
			}
			Xe = new ALine[h]();
			mem.h = h;
		}
	}
	ZeroMemory(Xe, mem.h * sizeof(ALine));
	ZeroMemory(Ye, mem.w * sizeof(YEdge));
	x_num = 0;
	y_num = 0;
}

EP::EP()
{
	x = y = xy = x2 = y2 = x2y = xy2 = x2y2 = x3 = y3 = x3y = xy3 = y4 = n = 0;
	minx = maxx = miny = maxy = 0;



	minx = maxx = miny = maxy = 0;


}
void EP::Init(ellipse& pupil)
{
	x = y = xy = x2 = y2 = x2y = xy2 = x2y2 = x3 = y3 = x3y = xy3 = y4 = n = 0;
	minx = maxx = miny = maxy = 0;


	pupil.roi = { 0,0,0,0 };
	pupil.eq = { 0,0,0,0,0 };
	pupil.center = { 0,0 };
	pupil.size = { 0,0 };
	pupil.axis = { 0,0 };
	pupil.slope = { 0,0 };
	pupil.percentx = 0;
	pupil.densityx = 0;
}
int EP::Store(int iix, int iiy)
{
	double ix, iy;

	ix = (double)iix;
	iy = (double)iiy;

	if (n < 1) {
		maxx = minx = ix;
		maxy = miny = iy;
	}

	if (maxx < ix) maxx = ix;
	if (minx > ix) minx = ix;
	if (maxy < iy) maxy = iy;
	if (miny > iy) miny = iy;

	x += ix;
	y += iy;
	xy += ix * iy;
	x2 += ix * ix;
	y2 += iy * iy;
	x2y += ix * ix * iy;
	xy2 += ix * iy * iy;
	x2y2 += ix * ix * iy * iy;
	x3 += ix * ix * ix;
	y3 += iy * iy * iy;
	x3y += ix * ix * ix * iy;
	xy3 += ix * iy * iy * iy;
	y4 += iy * iy * iy * iy;
	n = n + 1.0;
	return (int)n;
}
int EP_pibot_n(double* ans_mat, size_t n)
{
	double p, d, max, dumy;
	size_t i, j, k, s;

	for (k = 0; k < n; k++) {
		max = 0, s = k;
		for (j = k; j < n; j++) {
			if (fabs(*(ans_mat + k + j * (n + 1))) > max) {
				max = fabs(*(ans_mat + k + j * (n + 1))); s = j;
			}
		}
		if (max == 0) {
#ifdef CONWIN
			conwin.write(L"pibot max error!\n");
#endif
			return 0;
		}
		for (j = 0; j <= n; j++) {
			dumy = *(ans_mat + j + k * (n + 1));
			*(ans_mat + j + k * (n + 1)) = *(ans_mat + j + s * (n + 1));
			*(ans_mat + j + s * (n + 1)) = dumy;
		}


		p = *(ans_mat + k + k * (n + 1));
		if (p == 0) {
#ifdef CONWIN
			conwin.write(L"pibot p==0 error!\n");
#endif
			return 0;
		}
		for (j = k; j < (n + 1); j++) {
			*(ans_mat + j + k * (n + 1)) = (*(ans_mat + j + k * (n + 1))) / p;
		}

		for (i = 0; i < n; i++) {
			if (i != k) {
				d = *(ans_mat + k + i * (n + 1));
				for (j = k; j < (n + 1); j++) {
					*(ans_mat + j + i * (n + 1)) = *(ans_mat + j + i * (n + 1)) - d * (*(ans_mat + j + k * (n + 1)));
				}
			}
		}
	}
	return 1;
}
int EP::Calc(ellipse& pupil, int width, int height, int length)
{
	double ans[30];
	double a, b, c, d, e;
	double pcx, pcy, w, h;
	double a1, b1, c1, ax, ay, lx, ly, tmp, tmp1;
	double ellipseCheck;

	ans[0] = x2y2; ans[6 * 0 + 1] = xy3; ans[6 * 0 + 2] = x2y; ans[6 * 0 + 3] = xy2; ans[6 * 0 + 4] = xy; ans[6 * 0 + 5] = -1 * x3y;
	ans[6 * 1 + 0] = xy3; ans[6 * 1 + 1] = y4; ans[6 * 1 + 2] = xy2; ans[6 * 1 + 3] = y3; ans[6 * 1 + 4] = y2; ans[6 * 1 + 5] = -1 * x2y2;
	ans[6 * 2 + 0] = x2y; ans[6 * 2 + 1] = xy2; ans[6 * 2 + 2] = x2; ans[6 * 2 + 3] = xy; ans[6 * 2 + 4] = x; ans[6 * 2 + 5] = -1 * x3;
	ans[6 * 3 + 0] = xy2; ans[6 * 3 + 1] = y3; ans[6 * 3 + 2] = xy; ans[6 * 3 + 3] = y2; ans[6 * 3 + 4] = y; ans[6 * 3 + 5] = -1 * x2y;
	ans[6 * 4 + 0] = xy; ans[6 * 4 + 1] = y2; ans[6 * 4 + 2] = x; ans[6 * 4 + 3] = y; ans[6 * 4 + 4] = n; ans[6 * 4 + 5] = -1 * x2;

	if (::EP_pibot_n(ans, 5) == 1) {
		a = pupil.eq.a = ans[6 * 0 + 5];
		b = pupil.eq.b = ans[6 * 1 + 5];
		c = pupil.eq.c = ans[6 * 2 + 5];
		d = pupil.eq.d = ans[6 * 3 + 5];
		e = pupil.eq.e = ans[6 * 4 + 5];
		/*check parameter is ellipse?*/
		ellipseCheck = 4 * b - a * a;
		if (ellipseCheck > 0) {
			pcx = pupil.center.x = (a * d - 2 * b * c) / ellipseCheck;
			pcy = pupil.center.y = (a * c - 2 * d) / ellipseCheck;
			//		if ((4*b-a*a)>0.0001){
			//			pcy=(a*c-2*d)/(4*b-a*a);
			//			pcx=-1*(a*pcy+c)/2;
			a1 = a;
			b1 = b;
			c1 = pcx * pcx + a1 * pcx * pcy + b1 * pcy * pcy + c * pcx + d * pcy + e;
			if (a1 != 0) {
				tmp = (b1 - 1) / a1;
				ax = tmp + sqrt(tmp * tmp + 1);
				ay = tmp - sqrt(tmp * tmp + 1);
				lx = -1 * c1 * (ax * ax + 1) / (b1 * ax * ax + a1 * ax + 1);
				lx = sqrt(fabs(lx));
				ly = -1 * c1 * (ay * ay + 1) / (b1 * ay * ay + a1 * ay + 1);
				ly = sqrt(fabs(ly));
			}
			else {
				ax = 0;
				ay = 0;
				lx = sqrt(fabs(-1 * c1));
				ly = sqrt(fabs(-1 * c1 / b1));
			}
			if (lx > ly) {
				pupil.axis.l = lx; pupil.axis.s = ly; pupil.slope.s = ay; pupil.slope.l = ax;
			}
			else {
				pupil.axis.l = ly; pupil.axis.s = lx; pupil.slope.s = ax; pupil.slope.l = ay;
			}

			tmp = a * a - 4 * b;
			tmp1 = (2 * a * d - 4 * b * c) * (2 * a * d - 4 * b * c) - 4 * (a * a - 4 * b) * (d * d - 4 * b * e);
			w = pupil.size.w = fabs(sqrt(tmp1) / tmp);
			tmp1 = (2 * a * c - 4 * d) * (2 * a * c - 4 * d) - 4 * (a * a - 4 * b) * (c * c - 4 * e);
			h = pupil.size.h = fabs(sqrt(tmp1) / tmp);

			pupil.roi.left = static_cast<LONG>((pcx - w / 2 + 0.5) - 1);
			pupil.roi.right = static_cast<LONG>((pcx + w / 2 + 0.5) + 1);
			pupil.roi.top = static_cast<LONG>((pcy - h / 2 + 0.5) - 1);
			pupil.roi.bottom = static_cast<LONG>((pcy + h / 2 + 0.5) + 1);
			if (pupil.roi.left < 0) pupil.roi.left = 0;
			if (pupil.roi.right < 0) pupil.roi.right = 0;
			if (pupil.roi.top < 0) pupil.roi.top = 0;
			if (pupil.roi.bottom < 0) pupil.roi.bottom = 0;
			if (pupil.roi.left > width) pupil.roi.left = width - 1;
			if (pupil.roi.right > width) pupil.roi.right = width - 1;
			if (pupil.roi.top > height) pupil.roi.top = height - 1;
			if (pupil.roi.bottom > height) pupil.roi.bottom = height - 1;

			pupil.percentx = (maxy - miny) * 100 / h;
			pupil.densityx = n * 100 / (h * 2 + length);
			return (int)n;
		}
	}
#ifdef CONWIN
	{
		wstringstream wss;
		wss << endl;
		wss << L"a= " << a << endl;
		wss << L"b= " << b << endl;
		wss << L"c= " << c << endl;;
		wss << L"d= " << d << endl;;
		wss << L"e= " << e << endl;;
		conwin.write(wss.str());
	}
	conwin.write(L"elipse check error!\n");
#endif
	minx = maxx = miny = maxy = 0;
	pupil.roi = { 0,0,0,0 };
	pupil.eq = { 0,0,0,0,0 };
	pupil.center = { 0,0 };
	pupil.size = { 0,0 };
	pupil.axis = { 0,0 };
	pupil.slope = { 0,0 };
	pupil.percentx = 0;
	pupil.densityx = 0;
	return 0;
}

//bool l_cmp(const struct RefPoint& p, const struct RefPoint& q)
//{
//	return p.length < q.length;
//}
//
//bool a_cmp(const struct RefPoint& p, const struct RefPoint& q)
//{
//	return p.amount > q.amount;
//}


EyeImage::EyeImage()
{
	pupil.roi = { 0,0,0,0 };
	pupil.eq = { 0,0,0,0,0 };
	pupil.center = { 0,0 };
	pupil.size = { 0,0 };
	pupil.axis = { 0,0 };
	pupil.slope = { 0,0 };
	pupil.percentx = 0;
	pupil.densityx = 0;

	ref.pos = { 0,0 };
	ref.range = { 0,0,0,0 };
	ref.size = { 0,0 };

}
EyeImage::~EyeImage()
{

}
int EyeImage::GetIntensityRef(size_t x, size_t y)
{
	unsigned char* pos;
	if ((x < 1) || (y < 1) || (x >= ci.w - 1) || (y >= ci.h - 1)) return 0;
	pos = ci.img + x + y* ci.w;
	return *pos;
}
int EyeImage::Median3x3(size_t x, size_t y)
{
	int a[9];
	unsigned char* pos;
	if ((x < 1) || (y < 1) || (x >= ci.w-1) || (y >= ci.h-1)) return ImgParam.RefThreshold;
	pos = ci.img + x - 1 + (y - 1) * ci.w;

	a[0] = *pos++;
	a[1] = *pos++;
	a[2] = *pos;
	pos += ci.w;
	a[3] = *pos--;
	a[4] = *pos--;
	a[5] = *pos;
	pos += ci.w;
	a[6] = *pos++;
	a[7] = *pos++;
	a[8] = *pos;

	sort(a, a + 9);
	return a[4];
}
int EyeImage::Average3x3(size_t x, size_t y)
{
	int sum;
	unsigned char* pos;
	if ((x < 1) || (y < 1) || (x >= ci.w-1) || (y >= ci.h-1)) return 0;
	pos = ci.img + x - 1 + (y - 1) * ci.w;

	sum= *pos++;
	sum += *pos++;
	sum += *pos;
	pos += ci.w;
	sum += *pos--;
	sum += *pos--;
	sum += *pos;
	pos += ci.w;
	sum += *pos++;
	sum += *pos++;
	sum += *pos;

	return sum/9;
}

int EyeImage::SearchResion(void)
{
	int x, y, rx, ry, i, amount, index;
	size_t rw, rh;
	int sx, sy, ex, ey;

	sx = ImgParam.roi.left;
	ex = ImgParam.roi.right;
	sy = ImgParam.roi.top;
	ey = ImgParam.roi.bottom;

	if (sx <= 0) sx = 1;
	if (ex >= ci.w) ex = ci.w - 1;
	if (sy <= 0) sy = 1;
	if (ey >= ci.h) ey = ci.h - 1;

	rw = (ex - sx) / ReduceImageStep;
	rh = (ey - sy) / ReduceImageStep;

	ep.Init(pupil);

	if ((rw == 0) || (rh == 0)) {
#ifdef CONWIN
		conwin.write(L"(rw == 0) || (rh == 0)");
#endif
		return false;
	}
	pcl.SetArray(static_cast<int>(rw), static_cast<int>(rh));


	for (ry = 0, y = sy; ry < pcl.rsize.h; ry++, y += ReduceImageStep) {
		for (rx = 0, x = sx; rx < pcl.rsize.w; rx++, x += ReduceImageStep) {
			if ((y >= ey) || (x >= ex) || Median3x3(x, y) > ImgParam.PupilThreshold) {
				pcl.labelimgPtr[rx + ry * pcl.rsize.w] = 0;
			}
			else {
				pcl.CheckPoint(rx, ry);
			}
		}
	}
	for (i = 1, amount = 0, index = 0; i <= pcl.nlabel; i++) {
		if (pcl.labelPtr[i].co > amount) {
			amount = pcl.labelPtr[i].co;
			index = i;
		}
	}
	if (amount < 32) {
#ifdef CONWIN
		conwin.write(L"amount<32");
#endif
		return false;
	}
	for (y = pcl.labelPtr[index].t; y <= pcl.labelPtr[index].b; y++) {
		for (x = pcl.labelPtr[index].l; x <= pcl.labelPtr[index].r; x++)
			if (pcl.labelimgPtr[x + y * pcl.rsize.w] == index) {
				ep.Store(sx + x * ReduceImageStep, sy + y * ReduceImageStep);
				break;
			}
		for (x = pcl.labelPtr[index].r; x > pcl.labelPtr[index].l; x--) {
			if (pcl.labelimgPtr[x + y * pcl.rsize.w] == index) {
				ep.Store(sx + x * ReduceImageStep, sy + y * ReduceImageStep);
				break;
			}
		}
	}
	ep.Calc(pupil,ci.w, ci.h);
	pupil.roi.left -= ReduceImageStep;
	pupil.roi.top -= ReduceImageStep;
	if (pupil.roi.left < sx) pupil.roi.left = sx;
	if (pupil.roi.top < sy) pupil.roi.top = sy;
	pupil.roi.right += ReduceImageStep;
	pupil.roi.bottom += ReduceImageStep;
	if (pupil.roi.right >= ex) pupil.roi.right = ex - 1;
	if (pupil.roi.bottom >= ey) pupil.roi.bottom = ey - 1;

	if ((pupil.size.h >= ci.h) || (pupil.size.w >= ci.w)) {

#ifdef CONWIN
		conwin.write(L"pupil.size.h>=ci.h or pupil.size.w >= ci.w");
#endif
		ep.Init(pupil);
		return false;
	}

	if ((pupil.size.h <= 0) || (pupil.size.w <= 0)) {
#ifdef CONWIN
		conwin.write(L"(pupil.size.h <= 0) || (pupil.size.w <= 0)");
#endif
		ep.Init(pupil);
		return false;
	}


	return 1;
}

int EyeImage::GetXEdgeL(int& x, int y)
{
	int position, i, sx, ex, ret;
	ret = 0;

	if (Median3x3(x, y) > ImgParam.PupilThreshold) {
		ex = (ImgParam.roi.right > (x + ImgParam.SearchEdgeLength)) ? (x + ImgParam.SearchEdgeLength) : ImgParam.roi.right;
		for (i = x + 1; i < ex; i++) {
			if (Median3x3(i, y) <= ImgParam.PupilThreshold) {
				ret = 1;
				break;
			}
		}
	}
	else {
		sx = (ImgParam.roi.left < (x - ImgParam.SearchEdgeLength)) ? (x - ImgParam.SearchEdgeLength) : ImgParam.roi.left;
		for (i = x - 1; i >= sx; i--) {
			if (Median3x3(i, y) > ImgParam.PupilThreshold) {
				i++;
				ret = 1;
				break;
			}
		}
	}
	if (ret == 0) {
		return 0;
	}

	position = ((i + PreShift) < ImgParam.roi.right) ? (i + PreShift) : ImgParam.roi.right;
	if ((position - ImgParam.SearchEdgeLength) < ImgParam.roi.left) {
		position = ImgParam.roi.left + ImgParam.SearchEdgeLength;
	}
	{
		int maxDif = 0;
		int preData = Median3x3(static_cast<size_t>(position), y);
		int prePosition = position;
		int curData = 0;
		int difData = 0;
		int maxPosition = 0;
		position--;

		for (i = 1; i < ImgParam.SearchEdgeLength; i++, position--) {
			curData = Median3x3(static_cast<size_t>(position), y);
			difData = (curData - preData);
			if ((difData > maxDif) && curData < (ImgParam.PupilThreshold + ThresholdRange)) {
				maxDif = difData;
				maxPosition = prePosition;
			}
			prePosition = position;
			preData = curData;
		}
		x = maxPosition;
	}
	return 1;
}
int EyeImage::GetXEdgeR(int& x, int y)
{
	int position, i, sx, ex, ret;
	ret = 0;
	if (Median3x3(x, y) > ImgParam.PupilThreshold) {
		sx = (ImgParam.roi.left < (x - ImgParam.SearchEdgeLength)) ? (x - ImgParam.SearchEdgeLength) : ImgParam.roi.left;
		for (i = x - 1; i >= sx; i--) {
			if (Median3x3(i, y) <= ImgParam.PupilThreshold) {
				ret = 1;
				break;
			}
		}
	}
	else {
		ex = (ImgParam.roi.right > (x + ImgParam.SearchEdgeLength)) ? (x + ImgParam.SearchEdgeLength) : ImgParam.roi.right;
		for (i = x + 1; i < ex; i++) {
			if (Median3x3(i, y) > ImgParam.PupilThreshold) {
				i--;
				ret = 1;
				break;
			}
		}
	}
	if (ret == 0) return 0;
	position = ((i - PreShift) > ImgParam.roi.left) ? (i - PreShift) : ImgParam.roi.left;
	if (position + ImgParam.SearchEdgeLength > ImgParam.roi.right) {
		position = ImgParam.roi.right - ImgParam.SearchEdgeLength;
	}
	{

		int maxDif = 0;
		int preData = Median3x3(static_cast<size_t>(position), y);
		int prePosition = position;
		int curData = 0;
		int difData = 0;
		int maxPosition = 0;
		position++;

		for (i = 1; i < ImgParam.SearchEdgeLength; i++, position++) {
			curData = Median3x3(static_cast<size_t>(position), y);
			difData = (curData - preData);
			if ((difData > maxDif) && curData < (ImgParam.PupilThreshold + ThresholdRange)) {
				maxDif = difData;
				maxPosition = prePosition;
			}
			prePosition = position;
			preData = curData;
		}
		x = maxPosition;
	}
	return 1;
}
int EyeImage::GetYEdgeT(int x, int& y)
{
	int i, sy, ey, ret, position;
	ret = 0;

	if (Median3x3(x, y) > ImgParam.PupilThreshold) {
		ey = (ImgParam.roi.bottom > (y + ImgParam.SearchEdgeLength)) ? (y + ImgParam.SearchEdgeLength) : ImgParam.roi.bottom;
		for (i = y + 1; i < ey; i++) {
			if (Median3x3(x, i) <= ImgParam.PupilThreshold) {
				ret = 1;
				break;
			}
		}
	}
	else {
		sy = (ImgParam.roi.top < (y - ImgParam.SearchEdgeLength)) ? (y - ImgParam.SearchEdgeLength) : ImgParam.roi.top;
		for (i = y - 1; i >= sy; i--) {
			if ((Median3x3(x, i) > ImgParam.PupilThreshold)) {
				i++;
				ret = 1;
				break;
			}
		}
	}
	if (ret == 0) return 0;

	position = ((i + PreShift) < ImgParam.roi.bottom) ? (i + PreShift) : ImgParam.roi.bottom;
	if ((position - ImgParam.SearchEdgeLength) < ImgParam.roi.top) {
		position = ImgParam.roi.top + ImgParam.SearchEdgeLength;
	}
	{

		int maxDif = 0;
		int preData = Median3x3(x, static_cast<size_t>(position));
		int prePosition = position;
		int curData = 0;
		int difData = 0;
		int maxPosition = 0;
		position--;

		for (i = 1; i < ImgParam.SearchEdgeLength; i++, position--) {
			curData = Median3x3(x, static_cast<size_t>(position));
			difData = (curData - preData);
			if ((difData > maxDif) && curData < (ImgParam.PupilThreshold + ThresholdRange)) {
				maxDif = difData;
				maxPosition = prePosition;
			}
			prePosition = position;
			preData = curData;
		}
		y = maxPosition;
	}

	return 1;
}
int EyeImage::GetYEdgeB(int x, int& y)
{

	int i, sy, ey, ret, position;
	ret = 0;

	if (Median3x3(x, y) > ImgParam.PupilThreshold) {
		sy = (ImgParam.roi.top < (y - ImgParam.SearchEdgeLength)) ? (y - ImgParam.SearchEdgeLength) : ImgParam.roi.top;
		for (i = y - 1; i >= sy; i--) {
			if ((Median3x3(x, i) > ImgParam.PupilThreshold)) {
				ret = 1;
				break;
			}
		}

	}
	else {
		ey = (ImgParam.roi.bottom > (y + ImgParam.SearchEdgeLength)) ? (y + ImgParam.SearchEdgeLength) : ImgParam.roi.bottom;
		for (i = y + 1; i < ey; i++) {
			if (Median3x3(x, i) <= ImgParam.PupilThreshold) {
				i--;
				ret = 1;
				break;
			}
		}
	}
	if (ret == 0) return 0;

	position = ((i - PreShift) > ImgParam.roi.top) ? (i - PreShift) : ImgParam.roi.top;
	if ((position + ImgParam.SearchEdgeLength) > ImgParam.roi.bottom) {
		position = ImgParam.roi.bottom - ImgParam.SearchEdgeLength;
	}
	{

		int maxDif = 0;
		int preData = Median3x3(x, static_cast<size_t>(position));
		int prePosition = position;
		int curData = 0;
		int difData = 0;
		int maxPosition = 0;
		position++;

		for (i = 1; i < ImgParam.SearchEdgeLength; i++, position++) {
			curData = Median3x3(x, static_cast<size_t>(position));
			difData = (curData - preData);
			if ((difData > maxDif) && curData < (ImgParam.PupilThreshold + ThresholdRange)) {
				maxDif = difData;
				maxPosition = prePosition;
			}
			prePosition = position;
			preData = curData;
		}
		y = maxPosition;
	}

	return 1;
}

int EyeImage::FindXEdge(int y, int& xl, int& xr)
{
	if (!PredictXEdge(y, xl, xr))return 0;

	if (!GetXEdgeL(xl, y)) return 0;
	if (!GetXEdgeR(xr, y)) return 0;

	return 1;

}
bool EyeImage::PredictTopEdge(double x, int& y)
{
	double a1, b1, c1, d1, d2;
	int y1, y2;
	a1 = pupil.eq.b;
	b1 = pupil.eq.a * x + pupil.eq.d;
	c1 = x * x + pupil.eq.c * x + pupil.eq.e;
	d1 = b1 * b1 - 4 * a1 * c1;

	if (d1 < 0) return 0;
	d2 = sqrt(d1);
	a1 = 2 * a1;
	y1 = (int)((-b1 - d2) / a1 + 0.5);
	y2 = (int)((-b1 + d2) / a1 + 0.5);
	if (y1 < y2) y = y1;
	else y = y2;

	if ((y < ImgParam.roi.top) || (y >= ImgParam.roi.bottom)) {
		y = 0;
		return 0;
	}

	return 1;
}
bool EyeImage::PredictBottomEdge(double x, int& y)
{
	double a1, b1, c1, d1, d2;
	int y1, y2;
	a1 = pupil.eq.b;
	b1 = pupil.eq.a * x + pupil.eq.d;
	c1 = x * x + pupil.eq.c * x + pupil.eq.e;
	d1 = b1 * b1 - 4 * a1 * c1;

	if (d1 < 0) return false;
	d2 = sqrt(d1);
	a1 = 2 * a1;
	y1 = (int)((-b1 - d2) / a1 + 0.5);
	y2 = (int)((-b1 + d2) / a1 + 0.5);
	if (y1 > y2) y = y1;
	else y = y2;

	if ((y < ImgParam.roi.top) || (y >= ImgParam.roi.bottom)) {
		y = 0;
		return false;
	}

	return true;
}
bool EyeImage::FindYEdgeB(int x, int& y)
{
	int pyt, ydiff;

	if (!PredictBottomEdge(static_cast<double>(x), y))return false;
	pyt = y;

	if (!GetYEdgeB(x, y)) return false;

	ydiff = abs(pyt - y);
	if (ydiff > ImgParam.PermissiveLength) return false;

	return true;
}
bool EyeImage::FindYEdgeT(int x, int& y)
{
	int pyt, ydiff;

	if (!PredictTopEdge(static_cast<double>(x), y))return false;
	pyt = y;
	if (!GetYEdgeT(x, y)) return false;

	ydiff = abs(pyt - y);
	if (ydiff > ImgParam.PermissiveLength) return false;
	return true;
}
bool EyeImage::PredictXEdge(double y, int& xl, int& xr)
{
	double a1, b1, c1, d1, d2;
	a1 = 1.0;
	b1 = pupil.eq.a * y + pupil.eq.c;
	c1 = pupil.eq.b * y * y + pupil.eq.d * y + pupil.eq.e;
	d1 = b1 * b1 - 4 * a1 * c1;
	if (d1 < 0) {

		return 0;
	}
	d2 = sqrt(d1);
	a1 = 2 * a1;
	xl = static_cast<int>((-1 * b1 - d2) / a1 + 0.5);
	xr = static_cast<int>((-1 * b1 + d2) / a1 + 0.5);

	if ((xl < ImgParam.roi.left) || (xr < ImgParam.roi.left) || (xl >= ImgParam.roi.right) || (xr >= ImgParam.roi.right)) {
		xl = xr = 0;

		return false;
	}
	return true;
}

int EyeImage::CalcEllipseShapeX(void)
{
	int x,y, xl, xr, sy, ey, i, j;
	int angle;
//	int max_angle;
	int max_y, max_y_sx, max_y_ex, yb, min_y, min_y_sx, min_y_ex, yt, length;

	int ang[181];
	int num, max, miss_num;
	int ynum{};
	double dangle_sum, dangle_pos, dangle_num;
	pe.Set(ci.w, ci.h);
	length = 0;

	ZeroMemory(ang, 181 * sizeof(int));

	for (sy = ey = num = pe.x_num=0, y = pupil.roi.top; (y <= pupil.roi.bottom) && (num < ci.h); y++) {
		if (FindXEdge(y, xl, xr)) {
			if (sy == 0) sy = y;
			if (ey < y)  ey = y;
			pe.Xe[num].le.x = xl;
			pe.Xe[num].re.x = xr;
			pe.Xe[num].le.y = y;
			pe.Xe[num].re.y = y;
			pe.Xe[num].ce.x = (static_cast<double>(xl) + static_cast<double>(xr)) / 2.0;
			pe.Xe[num].ce.y = y;
			num++;
			pe.x_num++;
		}
	}

	if (num <= 5) {
#ifdef CONWIN
		conwin.write(L"(num <= 5)\n");
#endif
		ep.Init(pupil);
		return 0;
	}

	for (i = 0; i < num - 1; i++) {
		for (j = i + 1; j < num; j++) {
			angle = (int)(atan2(pe.Xe[j].ce.y - pe.Xe[i].ce.y, pe.Xe[j].ce.x - pe.Xe[i].ce.x) * 180 / M_PI + 0.5);
			if (angle >= 180) angle -= 180;
			if (angle < 0) angle += 180;
			if ((angle >= 0) && (angle < 180)) {
				pe.Xe[i].ang[angle]++;
				pe.Xe[j].ang[angle]++;
				ang[angle]++;
			}
		}
	}

	//for (i = 1, max = ang[0], max_angle = 0; i < 180; i++) {
	//	if (max < ang[i]) {
	//		max_angle = i;
	//		max = ang[i];
	//	}
	//}


  
	for (i = 0; i < num; i++) {
		max = pe.Xe[i].ang[0];
		pe.Xe[i].AngMax = 0;
		for (j = 1; j < 180; j++) {
			if (max < pe.Xe[i].ang[j]) {
				pe.Xe[i].AngMax = j;
				max = pe.Xe[i].ang[j];
			}
		}
	}

	vector <AngleFreq> ang_vec;
	AngleFreq af;
	for (i = 0; i < 180; i++) {
		af.angle = i;
		af.freq = ang[i];
		ang_vec.push_back(af);
	}
	std::sort(ang_vec.begin(), ang_vec.end(), [](const struct AngleFreq& p, const struct AngleFreq& q) {return p.freq > q.freq; });

	{
		double a_max = ang_vec[0].angle;
		double a_frq = ang_vec[0].freq;
		dangle_sum = ang_vec[0].angle * ang_vec[0].freq;
		dangle_num = ang_vec[0].freq;
		for (i = 1; i < 180; i++) {
			if (ang_vec[i].freq > ang_vec[0].freq * 9 / 10) {
				dangle_sum += ang_vec[i].freq * ang_vec[i].angle;
				dangle_num += ang_vec[i].freq;
			}
			else {
				break;
			}
		}
		if (dangle_num != 0) {
			dangle_pos = dangle_sum / dangle_num;
		}
	}


	ep.Init(pupil);
	for (i = 0; i < num; i++) {
		if (fabs(static_cast<double>(pe.Xe[i].AngMax) - dangle_pos) <= static_cast<double>(ImgParam.PermissiveAngle)) {
			ep.Store(pe.Xe[i].le.x, pe.Xe[i].le.y);
			ep.Store(pe.Xe[i].re.x, pe.Xe[i].re.y);
		}
	}
	ep.Calc(pupil,ci.w, ci.h);



	for (i = 0, miss_num = 0; i < num; i++) {
		y = pe.Xe[i].le.y;
		if (PredictXEdge(y, xl, xr)) {
			if (abs(pe.Xe[i].le.x - xl) > ImgParam.PermissiveLength) {
				pe.Xe[i].errflag.le = true;
				miss_num++;
			}
			if (abs(pe.Xe[i].re.x - xr) > ImgParam.PermissiveLength) {
				pe.Xe[i].errflag.re = true;
				miss_num++;
				continue;
			}
		}
	}

	ep.Init(pupil);
	max_y = max_y_sx = max_y_ex = 0;
	min_y = min_y_sx = min_y_ex = ci.h;
	for (i = 0; i < num; i++) {
		if (fabs(static_cast<double>(pe.Xe[i].AngMax) - dangle_pos) <= static_cast<double>(ImgParam.PermissiveAngle) && (pe.Xe[i].errflag.le == false) && (pe.Xe[i].errflag.re == false)) {
			ep.Store(pe.Xe[i].le.x, pe.Xe[i].le.y);
			ep.Store(pe.Xe[i].re.x, pe.Xe[i].re.y);

			if (max_y < pe.Xe[i].le.y) {
				max_y = pe.Xe[i].le.y;
				max_y_sx = pe.Xe[i].le.x;
				max_y_ex = pe.Xe[i].re.x;
			}
			if (min_y > pe.Xe[i].le.y) {
				min_y = pe.Xe[i].le.y;
				min_y_sx = pe.Xe[i].le.x;
				min_y_ex = pe.Xe[i].re.x;
			}
		}
		else {
			pe.Xe[i].errflag.le = true;
			pe.Xe[i].errflag.re = true;
		}
	}
	ep.Calc(pupil,ci.w, ci.h);

	if ((pupil.densityx < 10) || (pupil.percentx > 130) || (pupil.percentx < 10)) {
#ifdef CONWIN
		{
			wstringstream wss;
			wss << endl;
			wss << L"pupil.densityx (< 10)        :" << pupil.densityx << endl;
			wss << L"pupil.percentx (<10 or >130) :" << pupil.percentx << endl;
			wss << L"pe.x_num                        :" << pe.x_num << endl;
			wss << L"ep.size.w                       :" << pupil.size.w << endl;
			wss << L"ep.size.h                       :" << pupil.size.h << endl;
			wss << L"dangle_pos                      :" << dangle_pos << endl;
			for (int i = 0; i < pe.x_num; i++) {
				wss << pe.Xe[i].le.x << "," << pe.Xe[i].le.y << "," << static_cast<double>(pe.Xe[i].AngMax) - dangle_pos  <<"," << pe.Xe[i].errflag.le << endl;
				wss << pe.Xe[i].re.x << "," << pe.Xe[i].re.y << "," << pe.Xe[i].AngMax              <<"," << pe.Xe[i].errflag.re << endl;
			}
			wss << endl;
			wss << "angle,freq" << endl;
			for (int i = 0; i < 180; i++) {
				wss << i << "," << ang[i] << endl;
			}
			wss << "sort_angle,sort_freq" << endl;
			for (auto x : ang_vec) {
				wss << x.angle << "," << x.freq << endl;
			}
			conwin.write(wss.str());
		}
#endif
		ep.Init(pupil);
		return 0;
	}

	if ((miss_num * 100 / (num * 2)) > 50) {
#ifdef CONWIN
		conwin.write(L"(miss_num * 100 / (num * 2)\n");
#endif
		ep.Init(pupil);
		return 0;
	}

	length = 0;
	if (ImgParam.IsTopAcquisition && (min_y_sx != min_y_ex)) {
		length += (min_y_ex - min_y_sx - 1);
		for (i = min_y_sx + 1, ynum = pe.y_num=0; i < min_y_ex; i++) {
			if (FindYEdgeT(i, yt)) {
				ep.Store(i, yt);
				pe.Ye[ynum].x = i;
				pe.Ye[ynum].y = yt;
				pe.Ye[ynum].flag = true;
				ynum++;
				pe.y_num++;
			}
		}
		ep.Calc(pupil,ci.w, ci.h, length);
	}
	if (ImgParam.IsBottomAcquisition && (max_y_sx != max_y_ex)) {
		length += (max_y_ex - max_y_sx - 1);
		for (i = max_y_sx + 1; i < max_y_ex; i++) {
			if (FindYEdgeB(i, yb)) {
				ep.Store(i, yb);
				pe.Ye[ynum].x = i;
				pe.Ye[ynum].y = yb;
				pe.Ye[ynum].flag = true;
				ynum++;
				pe.y_num++;
			}
		}
		ep.Calc(pupil,ci.w, ci.h, length);
	}

	if (ImgParam.IsGleaningAcquisition) {
		for (i = 0; i < num; i++) {
			y = pe.Xe[i].le.y;
			if (PredictXEdge(y, xl, xr)) {
				if ((abs(pe.Xe[i].le.x - xl) < ImgParam.PermissiveLength) && (pe.Xe[i].errflag.le == true)) {
					pe.Xe[i].errflag.le = false;
					ep.Store(pe.Xe[i].le.x, pe.Xe[i].le.y);
				}
				if ((abs(pe.Xe[i].re.x - xr) < ImgParam.PermissiveLength) && (pe.Xe[i].errflag.re == true)) {
					pe.Xe[i].errflag.re = false;
					ep.Store(pe.Xe[i].re.x, pe.Xe[i].re.y);
				}
			}
		}
		ep.Calc(pupil,ci.w, ci.h, length);
	}

	pupil.UnderThresholdArea = 0;
	int yw;
	unsigned char* pos;
	for (i = 0; i < num; i++) {
		y = pe.Xe[i].le.y;
		yw = ci.w * y;
		if (PredictXEdge(y, xl, xr)) {
			for (x = xl; x <= xr; x++) {
				pos = ci.img + x + yw;
				if (*pos <= ImgParam.PupilThreshold) pupil.UnderThresholdArea+=1;
			}
		}
	}
	double pupil_area = pupil.axis.l * pupil.axis.s * M_PI;
	if (pupil_area != 0) {
		pupil.OpenRatio = pupil.UnderThresholdArea / pupil_area * 100;
	}
	else {
		pupil.OpenRatio = 0;
	}

	return 1;
}






int EyeImage::GetReflection(void)
{
	return GetReflection(static_cast<int>(pupil.center.x), static_cast<int>(pupil.center.y), ImgParam.roi, 1);
}

int EyeImage::GetReflection(int centerx, int centery, RECT& rc, int step)
{
	int i, j, amount, index;

	std::vector<struct RefPoint> v;
	struct RefPoint tmp;

	size_t rw, rh;
	int x, y, rx, ry;
	RECT r;

	ref.pos.x = 0;
	ref.pos.y = 0;
	ref.size.w = 0;
	ref.size.h = 0;
	ref.range.left = 0;
	ref.range.right = 0;
	ref.range.top = 0;
	ref.range.bottom = 0;

	r.left = rc.left <= 1 ? 1 : rc.left;
	r.right = rc.right >= ci.w ? ci.w - 2 : rc.right;
	r.top = rc.top <= 1 ? 1 : rc.top;
	r.bottom = rc.bottom >= ci.h ? ci.h - 2 : rc.bottom;

	rw = (r.right - r.left) / step;
	rh = (r.bottom - r.top) / step;

	if ((rw == 0) || (rh == 0)) {
		return false;
	}
	rcl.SetArray(static_cast<int>(rw), static_cast<int>(rh));

	for (ry = 0, y = r.top; ry < rcl.rsize.h; ry++, y += step) {
		for (rx = 0, x = r.left; rx < rcl.rsize.w; rx++, x += step) {
			if ((y >= r.bottom) || (x >= r.right) || GetIntensityRef(x, y) < ImgParam.RefThreshold) {
				rcl.labelimgPtr[rx + ry * rcl.rsize.w] = 0;
			}
			else {
				rcl.CheckPoint(rx, ry);
			}
		}
	}
	if (rcl.nlabel == 0) return false;
	for (i = 1, amount = 0; i <= rcl.nlabel; i++) {
		if (rcl.labelPtr[i].co * step > 5) {
			tmp.index = i;
			tmp.amount = rcl.labelPtr[i].co;
			tmp.length = abs(centerx - (r.left + step * (rcl.labelPtr[i].l + rcl.labelPtr[i].r) / 2));
			tmp.length += abs(centery - (r.top + step * (rcl.labelPtr[i].t + rcl.labelPtr[i].b) / 2));
			v.push_back(tmp);
			if (tmp.amount > amount)amount = tmp.amount;
		}
	}
	if (amount * step < 5) return false;
	//	std::sort(v.begin(), v.end(), l_cmp);
	std::sort(v.begin(), v.end(), [](const struct RefPoint& p, const struct RefPoint& q) {return p.length < q.length; });
	index = v[0].index;
	r.left = rcl.labelPtr[index].l * step + r.left;
	r.right = (rcl.labelPtr[index].r - rcl.labelPtr[index].l) * step + r.left;
	r.top = rcl.labelPtr[index].t * step + r.top;
	r.bottom = (rcl.labelPtr[index].b - rcl.labelPtr[index].t) * step + r.top;

	r.left -= step * 4;
	if (r.left <= 1) r.left = 1;
	r.right += step * 4;
	if (r.right >= ci.w - 1) r.right = ci.w - 2;
	r.top -= step * 4;
	if (r.top <= 1) r.top = 1;
	r.bottom += step * 4;
	if (r.bottom >= ci.w - 1) r.bottom = ci.w - 2;

	int rth = ImgParam.RefThreshold * 8 / 10;
	double num = 0, sum_x = 0, sum_y = 0, intensity;
	int min_x = ci.w, min_y = ci.h, max_x = 0, max_y = 0;
	for (j = r.top; j < r.bottom; j++) {
		for (i = r.left; i < r.right; i++) {
			intensity = static_cast<double>(Average3x3(i, j));
			if (intensity > rth) {
				sum_x += static_cast<double>(i) * intensity;
				sum_y += static_cast<double>(j) * intensity;
				num += intensity;
				if (min_x > i) min_x = i;
				if (min_y > j) min_y = j;
				if (max_x < i) max_x = i;
				if (max_y < j) max_y = j;
			}
		}
	}
	if (num <= 0) return false;

	ref.pos.x = sum_x / num;
	ref.pos.y = sum_y / num;
	ref.range.left = min_x;
	ref.range.right = max_x;
	ref.range.top = min_y;
	ref.range.bottom = max_y;
	ref.size.h = static_cast<double>(max_y) - static_cast<double>(min_y);
	ref.size.w = static_cast<double>(max_x) - static_cast<double>(min_x);
	return true;
}






int EyeImage::AnalyzeImage(void)
{

	if (SearchResion()) {
		if (CalcEllipseShapeX()) {
			GetReflection();
		}
	}

	return 0;
}

int EyeImage::AnalyzeImage(INT64 FrameID,ellipse &prev_pupil,reflect& prev_ref)
{
	if ((abs(ci.FrameID - FrameID) < 5)
		&& (prev_pupil.eq.b != 0)
		&& (prev_pupil.densityx > 30)
		&& (prev_pupil.center.x > ImgParam.roi.left)
		&& (prev_pupil.center.x < ImgParam.roi.right)
		&& (prev_pupil.center.y > ImgParam.roi.top)
		&& (prev_pupil.center.y < ImgParam.roi.bottom))
	{
		pupil = prev_pupil;
		if (CalcEllipseShapeX()) {
			//GetReflection(static_cast<int>(prev_ref.pos.x),static_cast<int>(prev_ref.pos.y), ImgParam.roi, 2);
			GetReflection(static_cast<int>(prev_pupil.center.x), static_cast<int>(prev_pupil.center.y), ImgParam.roi,1);
		}	
	}
	else {
		if (SearchResion()) {
			if (CalcEllipseShapeX()) {
				GetReflection();
			}
		}
	}


	return 0;
}