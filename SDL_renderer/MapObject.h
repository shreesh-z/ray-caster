#ifndef OBJECT_H
#define OBJECT_H
	
	class MapObject{
		public:
			double x, y, ang;
			double objDim;
			
			MapObject(double x_, double y_, double ang_, double objDim_);
			MapObject();
			void update(double x_, double y_, double ang_, double objDim_);
			void print();
			
	};
	
#endif