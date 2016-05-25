#include "main.h"

void ReadModel()
{

}

void AddInterLocking()
{

}

float Parabola_Function(float x)
{
	float A = InterLocking_Depth  / 3;
	return -InterLocking_Height / A / A * x * x + InterLocking_Height;
}

int main(int argc, char *argv[])
{
	MyMesh mesh;
	MyMesh output;
	OpenMesh::IO::read_mesh(mesh, "C:/Users/Dark/Desktop/Test.obj");
	OpenMesh::IO::read_mesh(output, "C:/Users/Dark/Desktop/Test.obj");

	// 刪除之前，一訂要加這個，不然不能刪除!!!!!
	mesh.request_vertex_status();
	mesh.request_edge_status();
	mesh.request_face_status();

	output.request_vertex_status();
	output.request_edge_status();
	output.request_face_status();

	MyMesh::Point *pointStack = new MyMesh::Point[3];
	QVector<MyMesh::Point> Face1;
	int index = 0;
	
	for (MyMesh::FaceIter f_it = mesh.faces_begin(); f_it != mesh.faces_end(); ++f_it)
	{
		index = 0;
		for (MyMesh::FaceVertexIter fv_it = mesh.fv_iter(f_it); fv_it.is_valid(); ++fv_it)
			pointStack[index++] = mesh.point(fv_it);
		if (pointStack[0][1] == pointStack[1][1] && pointStack[0][1] == pointStack[2][1] && pointStack[0][1] == 1)
		{

			if (Face1.length() == 0)
			{
				Face1.push_back(pointStack[0]);
				Face1.push_back(pointStack[1]);
				Face1.push_back(pointStack[2]);
			}
			else
				Face1.push_back(pointStack[2]);
			mesh.delete_face(f_it, false);
		}
	}
	mesh.garbage_collection();

	MyMesh::Point Center;
	Center[0] = (Face1[0][0] + Face1[1][0] + Face1[2][0] + Face1[3][0]) / 4;
	Center[1] = (Face1[0][1] + Face1[1][1] + Face1[2][1] + Face1[3][1]) / 4;
	Center[2] = (Face1[0][2] + Face1[1][2] + Face1[2][2] + Face1[3][2]) / 4;

	MyMesh::Point* TempPoint = new MyMesh::Point[4];
	TempPoint[0][0] = (Center[0] + (Center[0] - Face1[0][0] > 0) * -InterLocking_Size + (Center[0] - Face1[0][0] < 0) * InterLocking_Size) / 2;
	TempPoint[0][1] = Center[1] + (Center[1] - Face1[0][1] > 0) * -InterLocking_Size + (Center[1] - Face1[0][1] < 0) * InterLocking_Size;
	TempPoint[0][2] = (Center[2] + (Center[2] - Face1[0][2] > 0) * -InterLocking_Size + (Center[2] - Face1[0][2] < 0) * InterLocking_Size) / 2;

	TempPoint[1][0] = (Center[0] + (Center[0] - Face1[1][0] > 0) * -InterLocking_Size + (Center[0] - Face1[1][0] < 0) * InterLocking_Size) / 2;
	TempPoint[1][1] = Center[1] + (Center[1] - Face1[1][1] > 0) * -InterLocking_Size + (Center[1] - Face1[1][1] < 0) * InterLocking_Size;
	TempPoint[1][2] = (Center[2] + (Center[2] - Face1[1][2] > 0) * -InterLocking_Size + (Center[2] - Face1[1][2] < 0) * InterLocking_Size) / 2;

	TempPoint[2][0] = (Center[0] + (Center[0] - Face1[2][0] > 0) * -InterLocking_Size + (Center[0] - Face1[2][0] < 0) * InterLocking_Size) / 2;
	TempPoint[2][1] = Center[1] + (Center[1] - Face1[2][1] > 0) * -InterLocking_Size + (Center[1] - Face1[2][1] < 0) * InterLocking_Size;
	TempPoint[2][2] = (Center[2] + (Center[2] - Face1[2][2] > 0) * -InterLocking_Size + (Center[2] - Face1[2][2] < 0) * InterLocking_Size) / 2;

	TempPoint[3][0] = (Center[0] + (Center[0] - Face1[3][0] > 0) * -InterLocking_Size + (Center[0] - Face1[3][0] < 0) * InterLocking_Size) / 2;
	TempPoint[3][1] = Center[1] + (Center[1] - Face1[3][1] > 0) * -InterLocking_Size + (Center[1] - Face1[3][1] < 0) * InterLocking_Size;
	TempPoint[3][2] = (Center[2] + (Center[2] - Face1[3][2] > 0) * -InterLocking_Size + (Center[2] - Face1[3][2] < 0) * InterLocking_Size) / 2;

	#pragma region 產生面
	QVector<MyMesh::VertexHandle> TempVertex_Handle;
	TempVertex_Handle.push_back(mesh.add_vertex(Face1[0]));
	TempVertex_Handle.push_back(mesh.add_vertex(Face1[1]));
	TempVertex_Handle.push_back(mesh.add_vertex(TempPoint[0]));
	mesh.add_face(TempVertex_Handle.toStdVector());

	TempVertex_Handle.clear();
	TempVertex_Handle.push_back(mesh.add_vertex(Face1[1]));
	TempVertex_Handle.push_back(mesh.add_vertex(TempPoint[1]));
	TempVertex_Handle.push_back(mesh.add_vertex(TempPoint[0]));
	mesh.add_face(TempVertex_Handle.toStdVector());

	TempVertex_Handle.clear();
	TempVertex_Handle.push_back(mesh.add_vertex(Face1[0]));
	TempVertex_Handle.push_back(mesh.add_vertex(TempPoint[0]));
	TempVertex_Handle.push_back(mesh.add_vertex(Face1[2]));
	mesh.add_face(TempVertex_Handle.toStdVector());

	TempVertex_Handle.clear();
	TempVertex_Handle.push_back(mesh.add_vertex(TempPoint[0]));
	TempVertex_Handle.push_back(mesh.add_vertex(TempPoint[2]));
	TempVertex_Handle.push_back(mesh.add_vertex(Face1[2]));
	mesh.add_face(TempVertex_Handle.toStdVector());

	TempVertex_Handle.clear();
	TempVertex_Handle.push_back(mesh.add_vertex(TempPoint[2]));
	TempVertex_Handle.push_back(mesh.add_vertex(TempPoint[3]));
	TempVertex_Handle.push_back(mesh.add_vertex(Face1[2]));
	mesh.add_face(TempVertex_Handle.toStdVector());

	TempVertex_Handle.clear();
	TempVertex_Handle.push_back(mesh.add_vertex(Face1[2]));
	TempVertex_Handle.push_back(mesh.add_vertex(TempPoint[3]));
	TempVertex_Handle.push_back(mesh.add_vertex(Face1[3]));
	mesh.add_face(TempVertex_Handle.toStdVector());

	TempVertex_Handle.clear();
	TempVertex_Handle.push_back(mesh.add_vertex(Face1[1]));
	TempVertex_Handle.push_back(mesh.add_vertex(Face1[3]));
	TempVertex_Handle.push_back(mesh.add_vertex(TempPoint[3]));
	mesh.add_face(TempVertex_Handle.toStdVector());

	TempVertex_Handle.clear();
	TempVertex_Handle.push_back(mesh.add_vertex(TempPoint[1]));
	TempVertex_Handle.push_back(mesh.add_vertex(Face1[1]));
	TempVertex_Handle.push_back(mesh.add_vertex(TempPoint[3]));
	mesh.add_face(TempVertex_Handle.toStdVector());
	#pragma endregion
	#pragma  region 產生出起來的一塊
	float TempFloat = (InterLocking_Size - InterLocking_Gap) / 2;
	TempVertex_Handle.clear();
	TempVertex_Handle.push_back(mesh.add_vertex(TempPoint[0]));
	TempVertex_Handle.push_back(mesh.add_vertex(TempPoint[1]));
	TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[0][0], TempPoint[0][1] + InterLocking_Depth / 3, TempPoint[0][2])));
	mesh.add_face(TempVertex_Handle.toStdVector());

	TempVertex_Handle.clear();
	TempVertex_Handle.push_back(mesh.add_vertex(TempPoint[1]));
	TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[0][0] + TempFloat, TempPoint[0][1] + InterLocking_Depth / 3, TempPoint[0][2])));
	TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[0][0], TempPoint[0][1] + InterLocking_Depth / 3, TempPoint[0][2])));
	mesh.add_face(TempVertex_Handle.toStdVector());

	
	TempVertex_Handle.clear();
	TempVertex_Handle.push_back(mesh.add_vertex(TempPoint[1]));
	TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[1][0] - TempFloat, TempPoint[1][1] + InterLocking_Depth / 3, TempPoint[1][2])));
	TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[0][0], TempPoint[0][1] + InterLocking_Depth / 3, TempPoint[0][2])));
	mesh.add_face(TempVertex_Handle.toStdVector());

	TempVertex_Handle.clear();
	TempVertex_Handle.push_back(mesh.add_vertex(TempPoint[1]));
	TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[1][0], TempPoint[1][1] + InterLocking_Depth / 3, TempPoint[1][2])));
	TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[1][0] - TempFloat, TempPoint[1][1] + InterLocking_Depth / 3, TempPoint[1][2])));
	mesh.add_face(TempVertex_Handle.toStdVector());


	TempVertex_Handle.clear();
	TempVertex_Handle.push_back(mesh.add_vertex(TempPoint[2]));
	TempVertex_Handle.push_back(mesh.add_vertex(TempPoint[0]));
	TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[2][0], TempPoint[2][1] + InterLocking_Depth / 3, TempPoint[2][2])));
	mesh.add_face(TempVertex_Handle.toStdVector());

	TempVertex_Handle.clear();
	TempVertex_Handle.push_back(mesh.add_vertex(TempPoint[0]));
	TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[0][0], TempPoint[0][1] + InterLocking_Depth / 3, TempPoint[0][2])));
	TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[2][0], TempPoint[2][1] + InterLocking_Depth / 3, TempPoint[2][2])));
	mesh.add_face(TempVertex_Handle.toStdVector());

	
	TempVertex_Handle.clear();
	TempVertex_Handle.push_back(mesh.add_vertex(TempPoint[3]));
	TempVertex_Handle.push_back(mesh.add_vertex(TempPoint[2]));
	TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[3][0], TempPoint[3][1] + InterLocking_Depth / 3, TempPoint[3][2])));
	mesh.add_face(TempVertex_Handle.toStdVector());

	TempVertex_Handle.clear();
	TempVertex_Handle.push_back(mesh.add_vertex(TempPoint[2]));
	TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[3][0] - TempFloat, TempPoint[3][1] + InterLocking_Depth / 3, TempPoint[3][2])));
	TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[3][0], TempPoint[3][1] + InterLocking_Depth / 3, TempPoint[3][2])));
	mesh.add_face(TempVertex_Handle.toStdVector());

	TempVertex_Handle.clear();
	TempVertex_Handle.push_back(mesh.add_vertex(TempPoint[2]));
	TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[2][0] + TempFloat, TempPoint[2][1] + InterLocking_Depth / 3, TempPoint[2][2])));
	TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[3][0], TempPoint[3][1] + InterLocking_Depth / 3, TempPoint[3][2])));
	mesh.add_face(TempVertex_Handle.toStdVector());

	TempVertex_Handle.clear();
	TempVertex_Handle.push_back(mesh.add_vertex(TempPoint[2]));
	TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[2][0], TempPoint[2][1] + InterLocking_Depth / 3, TempPoint[2][2])));
	TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[2][0] + TempFloat, TempPoint[2][1] + InterLocking_Depth / 3, TempPoint[2][2])));
	mesh.add_face(TempVertex_Handle.toStdVector());


	TempVertex_Handle.clear();
	TempVertex_Handle.push_back(mesh.add_vertex(TempPoint[1]));
	TempVertex_Handle.push_back(mesh.add_vertex(TempPoint[3]));
	TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[1][0], TempPoint[1][1] + InterLocking_Depth / 3, TempPoint[1][2])));
	mesh.add_face(TempVertex_Handle.toStdVector());
	
	TempVertex_Handle.clear();
	TempVertex_Handle.push_back(mesh.add_vertex(TempPoint[3]));
	TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[3][0], TempPoint[3][1] + InterLocking_Depth / 3, TempPoint[3][2])));
	TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[1][0], TempPoint[1][1] + InterLocking_Depth / 3, TempPoint[1][2])));
	mesh.add_face(TempVertex_Handle.toStdVector());
	#pragma endregion
	#pragma region 產生半弧形的 卡榫
	float ThisX, NextX;
	for (float i = -InterLocking_Depth / 3; i < InterLocking_Depth / 3 * (1 + InterLocking_Depth_Scale); i += InterLocking_Height_Gap)
	{
		ThisX = Parabola_Function(i);
		NextX = Parabola_Function(i + InterLocking_Height_Gap);

		#pragma region 前面兩片
		TempVertex_Handle.clear();
		TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[0][0] - ThisX, TempPoint[0][1] + i + InterLocking_Depth * 2 / 3, TempPoint[0][2])));
		TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[0][0] + TempFloat, TempPoint[0][1] + i + InterLocking_Depth * 2 / 3, TempPoint[0][2])));
		TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[0][0] - NextX, TempPoint[0][1] + i + InterLocking_Height_Gap + InterLocking_Depth * 2 / 3, TempPoint[0][2])));
		mesh.add_face(TempVertex_Handle.toStdVector());

		TempVertex_Handle.clear();
		TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[0][0] + TempFloat, TempPoint[0][1] + i + InterLocking_Depth * 2 / 3, TempPoint[0][2])));
		TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[0][0] + TempFloat, TempPoint[0][1] + i + InterLocking_Height_Gap + InterLocking_Depth * 2 / 3, TempPoint[0][2])));
		TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[0][0] - NextX, TempPoint[0][1] + i + InterLocking_Height_Gap + InterLocking_Depth * 2 / 3, TempPoint[0][2])));
		mesh.add_face(TempVertex_Handle.toStdVector());

		TempVertex_Handle.clear();
		TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[1][0] - TempFloat, TempPoint[1][1] + i + InterLocking_Depth * 2 / 3, TempPoint[1][2])));
		TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[1][0] + ThisX, TempPoint[1][1] + i + InterLocking_Depth * 2 / 3, TempPoint[1][2])));
		TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[1][0] + NextX, TempPoint[1][1] + i + InterLocking_Height_Gap + InterLocking_Depth * 2 / 3, TempPoint[1][2])));
		mesh.add_face(TempVertex_Handle.toStdVector());

		TempVertex_Handle.clear();
		TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[1][0] - TempFloat, TempPoint[1][1] + i + InterLocking_Height_Gap + InterLocking_Depth * 2 / 3, TempPoint[1][2])));
		TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[1][0] - TempFloat, TempPoint[1][1] + i + InterLocking_Depth * 2 / 3, TempPoint[1][2])));
		TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[1][0] + NextX, TempPoint[1][1] + i + InterLocking_Height_Gap + InterLocking_Depth * 2 / 3, TempPoint[1][2])));
		mesh.add_face(TempVertex_Handle.toStdVector());
		#pragma endregion
		#pragma  region 後面兩片
		TempVertex_Handle.clear();
		TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[2][0] + TempFloat, TempPoint[2][1] + i + InterLocking_Depth * 2 / 3, TempPoint[2][2])));
		TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[2][0] - ThisX, TempPoint[2][1] + i + InterLocking_Depth * 2 / 3, TempPoint[2][2])));
		TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[2][0] - NextX, TempPoint[2][1] + i + InterLocking_Height_Gap + InterLocking_Depth * 2 / 3, TempPoint[2][2])));
		mesh.add_face(TempVertex_Handle.toStdVector());

		TempVertex_Handle.clear();
		TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[2][0] + TempFloat, TempPoint[2][1] + i + InterLocking_Height_Gap + InterLocking_Depth * 2 / 3, TempPoint[2][2])));
		TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[2][0] + TempFloat, TempPoint[2][1] + i + InterLocking_Depth * 2 / 3, TempPoint[2][2])));
		TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[2][0] - NextX, TempPoint[2][1] + i + InterLocking_Height_Gap + InterLocking_Depth * 2 / 3, TempPoint[2][2])));
		mesh.add_face(TempVertex_Handle.toStdVector());

		TempVertex_Handle.clear();
		TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[3][0] + ThisX, TempPoint[3][1] + i + InterLocking_Depth * 2 / 3, TempPoint[3][2])));
		TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[3][0] - TempFloat, TempPoint[3][1] + i + InterLocking_Depth * 2 / 3, TempPoint[3][2])));
		TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[3][0] + NextX, TempPoint[3][1] + i + InterLocking_Height_Gap + InterLocking_Depth * 2 / 3, TempPoint[3][2])));
		mesh.add_face(TempVertex_Handle.toStdVector());

		TempVertex_Handle.clear();
		TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[3][0] - TempFloat, TempPoint[3][1] + i + InterLocking_Depth * 2 / 3, TempPoint[3][2])));
		TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[3][0] - TempFloat, TempPoint[3][1] + i + InterLocking_Height_Gap + InterLocking_Depth * 2 / 3, TempPoint[3][2])));
		TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[3][0] + NextX, TempPoint[3][1] + i + InterLocking_Height_Gap + InterLocking_Depth * 2 / 3, TempPoint[3][2])));
		mesh.add_face(TempVertex_Handle.toStdVector());
		#pragma endregion

		#pragma  region 最外側兩個面
		TempVertex_Handle.clear();
		TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[2][0] - ThisX, TempPoint[2][1] + i + InterLocking_Depth * 2 / 3, TempPoint[2][2])));
		TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[0][0] - ThisX, TempPoint[0][1] + i + InterLocking_Depth * 2 / 3, TempPoint[0][2])));
		TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[2][0] - NextX, TempPoint[2][1] + i + InterLocking_Height_Gap + InterLocking_Depth * 2 / 3, TempPoint[2][2])));
		mesh.add_face(TempVertex_Handle.toStdVector());

		TempVertex_Handle.clear();
		TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[0][0] - ThisX, TempPoint[0][1] + i + InterLocking_Depth * 2 / 3, TempPoint[0][2])));
		TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[0][0] - NextX, TempPoint[0][1] + i + InterLocking_Height_Gap + InterLocking_Depth * 2 / 3, TempPoint[0][2])));
		TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[2][0] - NextX, TempPoint[2][1] + i + InterLocking_Height_Gap + InterLocking_Depth * 2 / 3, TempPoint[2][2])));
		mesh.add_face(TempVertex_Handle.toStdVector());

		TempVertex_Handle.clear();
		TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[1][0] + ThisX, TempPoint[1][1] + i + InterLocking_Depth * 2 / 3, TempPoint[1][2])));
		TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[3][0] + ThisX, TempPoint[3][1] + i + InterLocking_Depth * 2 / 3, TempPoint[3][2])));
		TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[3][0] + NextX, TempPoint[3][1] + i + InterLocking_Height_Gap + InterLocking_Depth * 2 / 3, TempPoint[3][2])));
		mesh.add_face(TempVertex_Handle.toStdVector());

		TempVertex_Handle.clear();
		TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[1][0] + NextX, TempPoint[1][1] + i + InterLocking_Height_Gap + InterLocking_Depth * 2 / 3, TempPoint[1][2])));
		TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[1][0] + ThisX, TempPoint[1][1] + i + InterLocking_Depth * 2 / 3, TempPoint[1][2])));
		TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[3][0] + NextX, TempPoint[3][1] + i + InterLocking_Height_Gap + InterLocking_Depth * 2 / 3, TempPoint[3][2])));
		mesh.add_face(TempVertex_Handle.toStdVector());
#pragma endregion
		#pragma  region 最內側兩個面
		TempVertex_Handle.clear();
		TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[0][0] + TempFloat, TempPoint[0][1] + i + InterLocking_Depth * 2 / 3, TempPoint[0][2])));
		TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[2][0] + TempFloat, TempPoint[2][1] + i + InterLocking_Depth * 2 / 3, TempPoint[2][2])));
		TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[2][0] + TempFloat, TempPoint[2][1] + i + InterLocking_Height_Gap + InterLocking_Depth * 2 / 3, TempPoint[2][2])));
		mesh.add_face(TempVertex_Handle.toStdVector());

		TempVertex_Handle.clear();
		TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[0][0] + TempFloat, TempPoint[0][1] + i + InterLocking_Height_Gap + InterLocking_Depth * 2 / 3, TempPoint[0][2])));
		TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[0][0] + TempFloat, TempPoint[0][1] + i + InterLocking_Depth * 2 / 3, TempPoint[0][2])));
		TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[2][0] + TempFloat, TempPoint[2][1] + i + InterLocking_Height_Gap + InterLocking_Depth * 2 / 3, TempPoint[2][2])));
		mesh.add_face(TempVertex_Handle.toStdVector());

		TempVertex_Handle.clear();
		TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[3][0] - TempFloat, TempPoint[3][1] + i + InterLocking_Depth * 2 / 3, TempPoint[3][2])));
		TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[1][0] - TempFloat, TempPoint[1][1] + i + InterLocking_Depth * 2 / 3, TempPoint[1][2])));
		TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[3][0] - TempFloat, TempPoint[3][1] + i + InterLocking_Height_Gap + InterLocking_Depth * 2 / 3, TempPoint[3][2])));
		mesh.add_face(TempVertex_Handle.toStdVector());

		TempVertex_Handle.clear();
		TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[1][0] - TempFloat, TempPoint[1][1] + i + InterLocking_Depth * 2 / 3, TempPoint[1][2])));
		TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[1][0]- TempFloat, TempPoint[1][1] + i + InterLocking_Height_Gap + InterLocking_Depth * 2 / 3, TempPoint[1][2])));
		TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[3][0]- TempFloat, TempPoint[3][1] + i + InterLocking_Height_Gap + InterLocking_Depth * 2 / 3, TempPoint[3][2])));
		mesh.add_face(TempVertex_Handle.toStdVector());
		#pragma endregion
	}
	#pragma region 最上面的兩個面 & 中間的一塊
	TempVertex_Handle.clear();
	TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[0][0] - Parabola_Function(InterLocking_Depth / 3 * (1 + InterLocking_Depth_Scale)), TempPoint[0][1] + InterLocking_Depth * (1 + InterLocking_Depth_Scale / 3), TempPoint[0][2])));
	TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[0][0] + TempFloat, TempPoint[0][1] + InterLocking_Depth * (1 + InterLocking_Depth_Scale / 3), TempPoint[0][2])));
	TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[2][0] - Parabola_Function(InterLocking_Depth / 3 * (1 + InterLocking_Depth_Scale)), TempPoint[2][1] + InterLocking_Depth * (1 + InterLocking_Depth_Scale / 3), TempPoint[2][2])));
	mesh.add_face(TempVertex_Handle.toStdVector());

	TempVertex_Handle.clear();
	TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[0][0] + TempFloat, TempPoint[0][1] + InterLocking_Depth * (1 + InterLocking_Depth_Scale / 3), TempPoint[0][2])));
	TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[2][0] + TempFloat, TempPoint[2][1] + InterLocking_Depth * (1 + InterLocking_Depth_Scale / 3), TempPoint[2][2])));
	TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[2][0] - Parabola_Function(InterLocking_Depth / 3 * (1 + InterLocking_Depth_Scale)), TempPoint[2][1] + InterLocking_Depth * (1 + InterLocking_Depth_Scale / 3), TempPoint[2][2])));
	mesh.add_face(TempVertex_Handle.toStdVector());

	TempVertex_Handle.clear();
	TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[1][0] - TempFloat, TempPoint[1][1] + InterLocking_Depth * (1 + InterLocking_Depth_Scale / 3), TempPoint[1][2])));
	TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[1][0] + Parabola_Function(InterLocking_Depth / 3 * (1 + InterLocking_Depth_Scale)), TempPoint[1][1] + InterLocking_Depth * (1 + InterLocking_Depth_Scale / 3), TempPoint[1][2])));
	TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[3][0] + Parabola_Function(InterLocking_Depth / 3 * (1 + InterLocking_Depth_Scale)), TempPoint[3][1] + InterLocking_Depth * (1 + InterLocking_Depth_Scale / 3), TempPoint[3][2])));
	mesh.add_face(TempVertex_Handle.toStdVector());

	TempVertex_Handle.clear();
	TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[1][0] - TempFloat, TempPoint[1][1] + InterLocking_Depth * (1 + InterLocking_Depth_Scale / 3), TempPoint[1][2])));
	TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[3][0] + Parabola_Function(InterLocking_Depth / 3 * (1 + InterLocking_Depth_Scale)), TempPoint[3][1] + InterLocking_Depth * (1 + InterLocking_Depth_Scale / 3), TempPoint[3][2])));
	TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[3][0] - TempFloat, TempPoint[3][1] + InterLocking_Depth * (1 + InterLocking_Depth_Scale / 3), TempPoint[3][2])));
	mesh.add_face(TempVertex_Handle.toStdVector());

	// 中間那塊
	TempVertex_Handle.clear();
	TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[0][0] + TempFloat, TempPoint[0][1] + InterLocking_Depth / 3, TempPoint[0][2])));
	TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[1][0] - TempFloat, TempPoint[1][1] + InterLocking_Depth / 3, TempPoint[1][2])));
	TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[3][0] - TempFloat, TempPoint[3][1] + InterLocking_Depth / 3, TempPoint[3][2])));
	mesh.add_face(TempVertex_Handle.toStdVector());

	TempVertex_Handle.clear();
	TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[3][0] - TempFloat, TempPoint[3][1] + InterLocking_Depth / 3, TempPoint[3][2])));
	TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[2][0] + TempFloat, TempPoint[2][1] + InterLocking_Depth / 3, TempPoint[2][2])));
	TempVertex_Handle.push_back(mesh.add_vertex(MyMesh::Point(TempPoint[0][0] + TempFloat, TempPoint[0][1] + InterLocking_Depth / 3, TempPoint[0][2])));
	mesh.add_face(TempVertex_Handle.toStdVector());
	#pragma endregion
	

	for (MyMesh::FaceIter f_it = output.faces_begin(); f_it != output.faces_end(); ++f_it)
	{
		index = 0;
		for (MyMesh::FaceVertexIter fv_it = output.fv_iter(f_it); fv_it.is_valid(); ++fv_it)
			pointStack[index++] = output.point(fv_it);
		if (pointStack[0][1] == pointStack[1][1] && pointStack[0][1] == pointStack[2][1] && pointStack[0][1] == 1)
		{

			if (Face1.length() == 0)
			{
				Face1.push_back(pointStack[0]);
				Face1.push_back(pointStack[1]);
				Face1.push_back(pointStack[2]);
			}
			else
				Face1.push_back(pointStack[2]);
			output.delete_face(f_it, false);
		}
	}
	output.garbage_collection();

	
	#pragma region 產生面
	delete TempPoint;
	TempPoint = new MyMesh::Point[4];					// 向外擴張點
	TempPoint[0][0] = (Center[0] + (Center[0] - Face1[0][0] > 0) * -(InterLocking_Size + InterLocking_Height) + (Center[0] - Face1[0][0] < 0) * (InterLocking_Size + InterLocking_Height)) / 2;
	TempPoint[0][1] = Center[1] + (Center[1] - Face1[0][1] > 0) * -(InterLocking_Size + InterLocking_Height) + (Center[1] - Face1[0][1] < 0) * (InterLocking_Size + InterLocking_Height);
	TempPoint[0][2] = (Center[2] + (Center[2] - Face1[0][2] > 0) * -(InterLocking_Size + InterLocking_Height) + (Center[2] - Face1[0][2] < 0) * (InterLocking_Size + InterLocking_Height)) / 2;

	TempPoint[1][0] = (Center[0] + (Center[0] - Face1[1][0] > 0) * -(InterLocking_Size + InterLocking_Height) + (Center[0] - Face1[1][0] < 0) * (InterLocking_Size + InterLocking_Height)) / 2;
	TempPoint[1][1] = Center[1] + (Center[1] - Face1[1][1] > 0) * -(InterLocking_Size + InterLocking_Height) + (Center[1] - Face1[1][1] < 0) * (InterLocking_Size + InterLocking_Height);
	TempPoint[1][2] = (Center[2] + (Center[2] - Face1[1][2] > 0) * -(InterLocking_Size + InterLocking_Height) + (Center[2] - Face1[1][2] < 0) * (InterLocking_Size + InterLocking_Height)) / 2;

	TempPoint[2][0] = (Center[0] + (Center[0] - Face1[2][0] > 0) * -(InterLocking_Size + InterLocking_Height) + (Center[0] - Face1[2][0] < 0) * (InterLocking_Size + InterLocking_Height)) / 2;
	TempPoint[2][1] = Center[1] + (Center[1] - Face1[2][1] > 0) * -(InterLocking_Size + InterLocking_Height) + (Center[1] - Face1[2][1] < 0) * (InterLocking_Size + InterLocking_Height);
	TempPoint[2][2] = (Center[2] + (Center[2] - Face1[2][2] > 0) * -(InterLocking_Size + InterLocking_Height) + (Center[2] - Face1[2][2] < 0) * (InterLocking_Size + InterLocking_Height)) / 2;

	TempPoint[3][0] = (Center[0] + (Center[0] - Face1[3][0] > 0) * -(InterLocking_Size + InterLocking_Height) + (Center[0] - Face1[3][0] < 0) * (InterLocking_Size + InterLocking_Height)) / 2;
	TempPoint[3][1] = Center[1] + (Center[1] - Face1[3][1] > 0) * -(InterLocking_Size + InterLocking_Height) + (Center[1] - Face1[3][1] < 0) * (InterLocking_Size + InterLocking_Height);
	TempPoint[3][2] = (Center[2] + (Center[2] - Face1[3][2] > 0) * -(InterLocking_Size + InterLocking_Height) + (Center[2] - Face1[3][2] < 0) * (InterLocking_Size + InterLocking_Height)) / 2;

	TempVertex_Handle.clear();
	TempVertex_Handle.push_back(output.add_vertex(Face1[0]));
	TempVertex_Handle.push_back(output.add_vertex(Face1[1]));
	TempVertex_Handle.push_back(output.add_vertex(TempPoint[0]));
	output.add_face(TempVertex_Handle.toStdVector());

	TempVertex_Handle.clear();
	TempVertex_Handle.push_back(output.add_vertex(Face1[1]));
	TempVertex_Handle.push_back(output.add_vertex(TempPoint[1]));
	TempVertex_Handle.push_back(output.add_vertex(TempPoint[0]));
	output.add_face(TempVertex_Handle.toStdVector());

	TempVertex_Handle.clear();
	TempVertex_Handle.push_back(output.add_vertex(Face1[0]));
	TempVertex_Handle.push_back(output.add_vertex(TempPoint[0]));
	TempVertex_Handle.push_back(output.add_vertex(Face1[2]));
	output.add_face(TempVertex_Handle.toStdVector());

	TempVertex_Handle.clear();
	TempVertex_Handle.push_back(output.add_vertex(TempPoint[0]));
	TempVertex_Handle.push_back(output.add_vertex(TempPoint[2]));
	TempVertex_Handle.push_back(output.add_vertex(Face1[2]));
	output.add_face(TempVertex_Handle.toStdVector());

	TempVertex_Handle.clear();
	TempVertex_Handle.push_back(output.add_vertex(TempPoint[2]));
	TempVertex_Handle.push_back(output.add_vertex(TempPoint[3]));
	TempVertex_Handle.push_back(output.add_vertex(Face1[2]));
	output.add_face(TempVertex_Handle.toStdVector());

	TempVertex_Handle.clear();
	TempVertex_Handle.push_back(output.add_vertex(Face1[2]));
	TempVertex_Handle.push_back(output.add_vertex(TempPoint[3]));
	TempVertex_Handle.push_back(output.add_vertex(Face1[3]));
	output.add_face(TempVertex_Handle.toStdVector());

	TempVertex_Handle.clear();
	TempVertex_Handle.push_back(output.add_vertex(Face1[1]));
	TempVertex_Handle.push_back(output.add_vertex(Face1[3]));
	TempVertex_Handle.push_back(output.add_vertex(TempPoint[3]));
	output.add_face(TempVertex_Handle.toStdVector());

	TempVertex_Handle.clear();
	TempVertex_Handle.push_back(output.add_vertex(TempPoint[1]));
	TempVertex_Handle.push_back(output.add_vertex(Face1[1]));
	TempVertex_Handle.push_back(output.add_vertex(TempPoint[3]));
	output.add_face(TempVertex_Handle.toStdVector());
	#pragma endregion
	#pragma region 產生往下凹的面
	TempVertex_Handle.clear();
	TempVertex_Handle.push_back(output.add_vertex(MyMesh::Point(TempPoint[0][0], TempPoint[0][1], TempPoint[0][2])));
	TempVertex_Handle.push_back(output.add_vertex(MyMesh::Point(TempPoint[1][0], TempPoint[1][1], TempPoint[1][2])));
	TempVertex_Handle.push_back(output.add_vertex(MyMesh::Point(TempPoint[0][0], TempPoint[0][1] - InterLocking_Depth, TempPoint[0][2])));
	output.add_face(TempVertex_Handle.toStdVector());

	TempVertex_Handle.clear();
	TempVertex_Handle.push_back(output.add_vertex(MyMesh::Point(TempPoint[1][0], TempPoint[1][1], TempPoint[1][2])));
	TempVertex_Handle.push_back(output.add_vertex(MyMesh::Point(TempPoint[1][0], TempPoint[1][1] - InterLocking_Depth, TempPoint[1][2])));
	TempVertex_Handle.push_back(output.add_vertex(MyMesh::Point(TempPoint[0][0], TempPoint[0][1] - InterLocking_Depth, TempPoint[0][2])));
	output.add_face(TempVertex_Handle.toStdVector());

	TempVertex_Handle.clear();
	TempVertex_Handle.push_back(output.add_vertex(MyMesh::Point(TempPoint[0][0], TempPoint[0][1], TempPoint[0][2])));
	TempVertex_Handle.push_back(output.add_vertex(MyMesh::Point(TempPoint[2][0], TempPoint[2][1] - InterLocking_Depth, TempPoint[2][2])));
	TempVertex_Handle.push_back(output.add_vertex(MyMesh::Point(TempPoint[2][0], TempPoint[2][1], TempPoint[2][2])));
	output.add_face(TempVertex_Handle.toStdVector());

	TempVertex_Handle.clear();
	TempVertex_Handle.push_back(output.add_vertex(MyMesh::Point(TempPoint[0][0], TempPoint[0][1], TempPoint[0][2])));
	TempVertex_Handle.push_back(output.add_vertex(MyMesh::Point(TempPoint[0][0], TempPoint[0][1] - InterLocking_Depth, TempPoint[0][2])));
	TempVertex_Handle.push_back(output.add_vertex(MyMesh::Point(TempPoint[2][0], TempPoint[2][1] - InterLocking_Depth, TempPoint[2][2])));
	output.add_face(TempVertex_Handle.toStdVector());

	TempVertex_Handle.clear();
	TempVertex_Handle.push_back(output.add_vertex(MyMesh::Point(TempPoint[3][0], TempPoint[3][1], TempPoint[3][2])));
	TempVertex_Handle.push_back(output.add_vertex(MyMesh::Point(TempPoint[2][0], TempPoint[2][1], TempPoint[2][2])));
	TempVertex_Handle.push_back(output.add_vertex(MyMesh::Point(TempPoint[2][0], TempPoint[2][1] - InterLocking_Depth, TempPoint[2][2])));
	output.add_face(TempVertex_Handle.toStdVector());

	TempVertex_Handle.clear();
	TempVertex_Handle.push_back(output.add_vertex(MyMesh::Point(TempPoint[3][0], TempPoint[3][1], TempPoint[3][2])));
	TempVertex_Handle.push_back(output.add_vertex(MyMesh::Point(TempPoint[2][0], TempPoint[2][1] - InterLocking_Depth, TempPoint[2][2])));
	TempVertex_Handle.push_back(output.add_vertex(MyMesh::Point(TempPoint[3][0], TempPoint[3][1] - InterLocking_Depth, TempPoint[3][2])));
	output.add_face(TempVertex_Handle.toStdVector());

	TempVertex_Handle.clear();
	TempVertex_Handle.push_back(output.add_vertex(MyMesh::Point(TempPoint[1][0], TempPoint[1][1], TempPoint[1][2])));
	TempVertex_Handle.push_back(output.add_vertex(MyMesh::Point(TempPoint[3][0], TempPoint[3][1], TempPoint[3][2])));
	TempVertex_Handle.push_back(output.add_vertex(MyMesh::Point(TempPoint[3][0], TempPoint[3][1] - InterLocking_Depth, TempPoint[3][2])));
	output.add_face(TempVertex_Handle.toStdVector());

	TempVertex_Handle.clear();
	TempVertex_Handle.push_back(output.add_vertex(MyMesh::Point(TempPoint[1][0], TempPoint[1][1], TempPoint[1][2])));
	TempVertex_Handle.push_back(output.add_vertex(MyMesh::Point(TempPoint[3][0], TempPoint[3][1] - InterLocking_Depth, TempPoint[3][2])));
	TempVertex_Handle.push_back(output.add_vertex(MyMesh::Point(TempPoint[1][0], TempPoint[1][1] - InterLocking_Depth, TempPoint[1][2])));
	output.add_face(TempVertex_Handle.toStdVector());
	#pragma endregion 
	#pragma region 產生下面的平面
	TempVertex_Handle.clear();
	TempVertex_Handle.push_back(output.add_vertex(MyMesh::Point(TempPoint[0][0], TempPoint[0][1] - InterLocking_Depth, TempPoint[0][2])));
	TempVertex_Handle.push_back(output.add_vertex(MyMesh::Point(TempPoint[1][0], TempPoint[1][1] - InterLocking_Depth, TempPoint[1][2])));
	TempVertex_Handle.push_back(output.add_vertex(MyMesh::Point(TempPoint[3][0], TempPoint[3][1] - InterLocking_Depth, TempPoint[3][2])));
	output.add_face(TempVertex_Handle.toStdVector());

	TempVertex_Handle.clear();
	TempVertex_Handle.push_back(output.add_vertex(MyMesh::Point(TempPoint[0][0], TempPoint[0][1] - InterLocking_Depth, TempPoint[0][2])));
	TempVertex_Handle.push_back(output.add_vertex(MyMesh::Point(TempPoint[3][0], TempPoint[3][1] - InterLocking_Depth, TempPoint[3][2])));
	TempVertex_Handle.push_back(output.add_vertex(MyMesh::Point(TempPoint[2][0], TempPoint[2][1] - InterLocking_Depth, TempPoint[2][2])));
	output.add_face(TempVertex_Handle.toStdVector());
	#pragma endregion
	OpenMesh::IO::write_mesh(mesh, "D:/a.obj");
	OpenMesh::IO::write_mesh(output, "D:/b.obj");
	//system("pause");
	return 0;
}

