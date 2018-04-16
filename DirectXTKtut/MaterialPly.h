#pragma once
using namespace std;

class MaterialPlySimple
{
	std::string name;
	string diffuse;
	string blend;	// none,test
	bool tile{ false };
	bool alphatocoverage{ false };

public:
	MaterialPlySimple();
	~MaterialPlySimple();

	static std::shared_ptr<MaterialPlySimple> CreateFromPak(ID3D11Device* device, LPCWSTR szPakFile, LPCSTR szMatFile);
private:
	static std::shared_ptr<MaterialPlySimple> CreateFromMTL(ID3D11Device* d3dDevice, const char* meshData, size_t dataSize);
};

class MaterialPlyBump :
	public MaterialPlySimple
{
	string specular;
	UINT32 color;

public:
	MaterialPlyBump();
	~MaterialPlyBump();
};

namespace MTL {
	class material {
		int Blend;
		int FakeReflection;
		int Modulate2x;
		int alphatocoverage;
		int amount;
		int blend;
		int bump;
		int bumpVolume;
		int color;
		class diffuse {
			int AlphaChannel;
			int Axis;
			int MipMap;
		};
		int diffuse1;
		int envamount;
		int envmap;
		int height;
		int mask;
		int parallax_scale;
		int period;
		int simple;
		int specular;
		int tile;
	};
};

namespace MDL {
	class Area {
		class Polygon2 {
			int Radius;
			int Vertex;
		};
		int Rotate;
	};
	class Extender {
		class Stuff {
			class camera {
				int fov;
				int horizon;
				int lookat;
				int origin;
			};
			class lights {
				class 0 {
					int ambient;
					int diffuse;
					int direction;
					int phi;
					int position;
					int theta;
					int type;
				};
				class 1 {
					int diffuse;
					int direction;
					int phi;
					int position;
					int theta;
					int type;
				};
				class 2 {
					int ambient;
					int diffuse;
					int direction;
					int position;
					int type;
				};
				class 3 {
					int position;
					int type;
				};
			};
		};
		class Stuff_primary {
			class camera {
				int fov;
				int horizon;
				int lookat;
				int origin;
			};
			class lights {
				class 0 {
					int ambient;
					int diffuse;
					int direction;
					int phi;
					int position;
					int theta;
					int type;
				};
				class 1 {
					int diffuse;
					int direction;
					int phi;
					int position;
					int theta;
					int type;
				};
				class 2 {
					int diffuse;
					int direction;
					int position;
					int type;
				};
				class 3 {
					int position;
					int type;
				};
			};
		};
		class Stuff_secondary {
			class camera {
				int fov;
				int horizon;
				int lookat;
				int origin;
			};
			class lights {
				class 0 {
					int direction;
					int phi;
					int position;
					int theta;
					int type;
				};
				class 1 {
					int direction;
					int phi;
					int position;
					int theta;
					int type;
				};
				class 2 {
					int direction;
					int position;
					int type;
				};
				class 3 {
					int position;
					int type;
				};
			};
		};
		class stuff {
			class camera {
				int fov;
				int horizon;
				int lookat;
				int origin;
			};
			class lights {
				class 0 {
					int ambient;
					int diffuse;
					int direction;
					int position;
					int type;
				};
				class 1 {
					int diffuse;
					int direction;
					int position;
					int type;
				};
				class 2 {
					int diffuse;
					int direction;
					int position;
					int type;
				};
				class 3 {
					int direction;
					int type;
				};
				class 4 {
					int position;
					int type;
				};
			};
		};
		class stuff_primary {
			class camera {
				int fov;
				int horizon;
				int lookat;
				int origin;
			};
			class lights {
				class 0 {
					int diffuse;
					int direction;
					int position;
					int type;
				};
				class 1 {
					int ambient;
					int diffuse;
					int direction;
					int position;
					int type;
				};
				class 2 {
					int diffuse;
					int position;
					int type;
				};
				class 3 {
					int direction;
					int type;
				};
				class 4 {
					int position;
					int type;
				};
			};
		};
		class stuff_secondary {
			class camera {
				int fov;
				int horizon;
				int lookat;
				int origin;
			};
			class lights {
				class 0 {
					int direction;
					int type;
				};
				class 1 {
					int direction;
					int type;
				};
				class 2 {
					int direction;
					int type;
				};
			};
		};
	};
	class Obstacle {
		class Circle2 {
			int Center;
			int Radius;
		};
		class Obb2 {
			int Axis;
			int Center;
			int Extent;
		};
		class Polygon2 {
			int Radius;
			int Vertex;
		};
		int Rotate;
		int Tags;
	};
	class Skeleton {
		class animation {
			class sequence {
				int autostart;
				int resume;
				int speed;
			};
		};
		class bone {
			int Color;
			class LODView {
				int VolumeView;
			};
			int Orientation;
			class VolumeView {
				int DecalTarget;
				int DecalTargetDynamic;
				int Ground;
				int Layer;
				int nocastshadows;
				int nogroupmesh;
			};
			class animation {
				class uv {
					int shiftV;
				};
			};
			class bone {
				int Color;
				class LODView {
					int VolumeView;
				};
				int Matrix34;
				int Orientation;
				int Position;
				class VolumeView {
					int DecalTarget;
					int Ground;
					int layer;
				};
				class bone {
					int Matrix34;
					int Position;
					int VolumeView;
					class bone {
						int Matrix34;
						int VolumeView;
						class bone {
							int Matrix34;
							int Position;
							int VolumeView;
							class bone {
								int Matrix34;
								int Position;
								class bone {
									int Matrix34;
									int Position;
									class bone {
										int Matrix34;
										int Position;
										int VolumeView;
										class bone {
											int Matrix34;
											int Position;
											int VolumeView;
											class bone {
												int Matrix34;
												class bone {
													int Matrix34;
													class bone {
														int Matrix34;
														class bone {
															int Matrix34;
															class bone {
																int Matrix34;
															};
															int parameters;
														};
														int parameters;
														int visibility;
													};
													int visibility;
												};
												int visibility;
											};
											int visibility;
										};
										int parameters;
										int visibility;
									};
									int parameters;
								};
								int limits;
								int parameters;
								int speed;
							};
							int parameters;
						};
						int limits;
						int parameters;
						int speed;
					};
					int limits;
					int parameters;
					int speed;
				};
				int limits;
				int parameters;
				int speed;
				int visibility;
			};
			int limits;
			int matrix34;
			int parameters;
			int speed;
			int visibility;
		};
		int extension;
	};
	class Volume {
		int Bone;
		int Box;
		int Cylinder;
		int Matrix34;
		int Polyhedron;
		int Position;
		int Sphere;
		int box;
	};
};
