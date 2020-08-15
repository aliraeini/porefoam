// Surface generator from 3D images

// Copyright (C) 2020  Ali Qaseminejad Raeini 

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

// For any queries, contact Ali Q. Raeini: email aliqasemi@gmail.com


//#include "Vctr.h"
//#include "Tnsr.h"
#include "voxelImage.h"
//#include "voxelImageML.h" //zeroGrad


#include "InputFile.h"

//#include "voxelSurface.h"


int usage()
{
	std::cout<<"\nvoxelToSurfMesh:\n converting 3D image file to  surface format"
		<<"\nusages: \n"
		<<"    voxelToSurfMesh inputMetaImage.mhd outPutFile.obj \n"
		<< std::endl;
	return -1;
}

//int debugLevel_ = 0;
#define MAIN
//#include "globals.cpp"
#include "voxelImageConvert.cpp"

#include "surfUtils.h"
surfMsh createSurface(InputFile& meshingDict,  const voxelImage & vxlImg, const int nVVs, const std::string& outputSurface);

int main(int argc, char *argv[])
{

	if(argc<2) return usage();

	std::string headerName(argv[1]);

    InputFile meshingDict(headerName, 0);
		meshingDict.echoKeywords(cout);

	if(headerName.size()<4) return usage();
	std::string outputSurface = (argc>2) ? string(argv[2]) : 
		meshingDict.getOr(headerName.substr(0,headerName.size()-4)+".obj", "outputSurface");



	voxelImage vimage(headerName);
	int3 n = vimage.size3();

	vimage.cropD(int3(0,0,0),vimage.size3(),1,1);	//		 XXXXXXXXXXXXXXXXXXXXXXXXXXXX

	//keepLargest0(vimage); //! CtrlF:isolated=254


	vimage.printInfo();

	//if(resetX0=='T' || resetX0=='t')		vimage.X0Ch()=-vimage.dx();


	//! solid phase should have the highest vv TODO: add multilabel
	int nVVs=0;  
	forAllvv_seq(vimage) if(vv<240) nVVs =max(nVVs,int(vv));
	cout<<" maxVxlvalue:"<<nVVs<<endl;
	forAllvp_(vimage) if(*vp>nVVs) *vp=nVVs; //! set isolateds to last vv, CtrlF:isolated=254
	++nVVs;
	cout<<"nVVs:"<<int(nVVs)<<endl;


	if (meshingDict.lookupOr("extractBoxBoundary",false))
	{
		cout<<"extracting box boundary"<<endl;
		const int
			Left  =nVVs+0,
			Right =nVVs+1,
			Bottom=nVVs+2,
			Top   =nVVs+3,
			Back  =nVVs+4,
			Front =nVVs+5;
			vimage.setSlice('i',0,     0+1*Left  );
			vimage.setSlice('i',n[0]+1,0+1*Right );
			vimage.setSlice('j',0,     0+1*Bottom);
			vimage.setSlice('j',n[1]+1,0+1*Top   );
			vimage.setSlice('k',0,     0+1*Back  );
			vimage.setSlice('k',n[2]+1,0+1*Front );
	}
	else
	{
		vimage.zeroGrad(1);
	}

	if (meshingDict.lookupOr("filterImage",true))
	{
		bool verbose=true;
		(cout<<" Filtering Image: ").flush();
		//modeNSames(vimage,2,verbose);
		(cout<<" .").flush();
		int maxitr=10;
		while(modeNSames(vimage,1,verbose) && (--maxitr)) (cout<<".").flush(); //was .mod(3)

		(cout<<"\n.").flush();
		maxitr=20;
		while(modeNSames(vimage,2,verbose) && (--maxitr)) (cout<<".").flush(); //was .mod(3) // without this the surface correction most likely would crash
	}
	else (cout<<"Warning, not filetring image is not a good idea").flush();



	vimage.printInfo();

	cout<<" createSurface { outputFileName: "<<outputSurface<<endl;

	surfMsh srfMsh = createSurface(meshingDict, vimage,nVVs, outputSurface);//         XXXXXXXXXXXXXXXXXXXXXXXXXXXX

	//piece<point> pointsp=piece<point>(srfMsh.points);

	smoothSurf(meshingDict, srfMsh.faces_bs, srfMsh.points);

	writeSurfaceFiles(srfMsh.faces_bs, srfMsh.points, outputSurface);
	writeMergeSurfaceFile(srfMsh.faces_bs, srfMsh.points, outputSurface);


	cout<< "end }" << endl;

	return 0;
}














