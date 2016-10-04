#include "Convection2d.h"

#define np 301 // number of points along single axis

int main(int argc, char **argv){
    
    physics    *phys = PhysicsCreate();
    structMesh *mesh = MeshCreate();
    double     **c   = ConcentrationCreate(mesh, phys);
    double     finalTime = 0.5;
    
    ConvectionSolve2d(mesh, phys, c, finalTime);
    
    double L2, Linf;
    NormErr(mesh, phys, finalTime, c, &L2, &Linf);
    printf("The Mesh Grid, [%d, %d]\n", mesh->ndim1, mesh->ndim2);
    printf("The Norm Error, L2=%f, Linf=%f\n", L2, Linf);
    
    free(phys);
    Matrix_free(c);
    MeshFree(mesh);
    return 0;
}

/**
 * @brief
 * Allocate Memory for Mesh strucutre.
 *
 * @return
 * name     | type     | description of value
 * -------- |----------|----------------------
 * mesh  | structMesh* | pointer to the mesh structure
 *
 */

structMesh* MeshCreate(){
    
    structMesh *mesh = (structMesh*)calloc(1, sizeof(structMesh));
    double xmin, xmax, ymin, ymax;
    int dim1, dim2;
    
    // initialize the mesh
    xmin = 0.0; xmax = 2.0;
    ymin = 0.0; ymax = 2.0;
    
    mesh->ndim1 = np; // No. of points along x coordinate
    mesh->ndim2 = np; // No. of points along y coordinate
    mesh->dx = (xmax - xmin)/(double)(np - 1.0);
    mesh->dy = (ymax - ymin)/(double)(np - 1.0);
    
    mesh->x = Matrix_create(mesh->ndim1, mesh->ndim2);
    mesh->y = Matrix_create(mesh->ndim1, mesh->ndim2);
    
    // initialize the condition
    for (dim1=0; dim1<mesh->ndim1; dim1++) {
        for (dim2=0; dim2<mesh->ndim2; dim2++) {
            mesh->x[dim1][dim2] = dim2*mesh->dx;
            mesh->y[dim1][dim2] = dim1*mesh->dy;
        }
    }
    return mesh;
}

/**
 * @brief
 * Deallocate Memory for Mesh strucutre.
 */
void MeshFree(structMesh *mesh){
    Matrix_free(mesh->x);
    Matrix_free(mesh->y);
    return;
}

/**
 * @brief
 * Allocate Memory for concentration variable.
 */
double** ConcentrationCreate(structMesh *mesh, physics *phys){
    
    double **c = Matrix_create(mesh->ndim1, mesh->ndim2);
    double x0  = phys->x0;
    double y0  = phys->y0;
    double **x = mesh->x;
    double **y = mesh->y;
    double Dx  = phys->Dx;
    double Dy  = phys->Dy;
    
    int dim1, dim2;
    // initial condiation
    for (dim1=0; dim1< mesh->ndim1; dim1++) {
        for (dim2=0; dim2< mesh->ndim2; dim2++)
            c[dim1][dim2] = exp(-pow(x[dim1][dim2] - x0, 2.0)/Dx
                                - pow(y[dim1][dim2] - y0, 2.0)/Dy);
    }
    return c;
}

void ConcentrationFree(double **c){
    Matrix_free(c);
    return;
}

/**
 * @brief
 * Allocate Memory and Initizlize for Physics Structure.
 */
physics* PhysicsCreate(){
    
    physics *phys = (physics *)calloc(1, sizeof(physics));
    // initialize physics constant
    phys->u  = 1.0;
    phys->v  = 1.0;
    phys->Dx = 0.01;
    phys->Dy = 0.01;
    phys->x0 = 0.5;
    phys->y0 = 0.5;

    return phys;
}

/**
 * @brief
 * Deallocate Memory for physics structure.
 */
void PhysicsFree(physics *phys){
    
    free(phys);
    return;
}

/**
 * @brief
 * Exact Solution of Convection2d Problem.
 */
void ExactSol(structMesh *mesh, physics *phys, double time, double **c_exa){
    int dim1, dim2;
    double cx, cy;
    double **x = mesh->x;
    double **y = mesh->y;
    double u   = phys->u;
    double v   = phys->v;
    double x0  = phys->x0;
    double y0  = phys->y0;
    
    for (dim1=0; dim1<mesh->ndim1; dim1++) {
        for (dim2=0; dim2<mesh->ndim2; dim2++) {
            cx = exp( -pow( x[dim1][dim2]-x0-u*time, 2.0 )/( phys->Dx*(4.0*time+1) ) );
            cy = exp( -pow( y[dim1][dim2]-y0-v*time, 2.0 )/( phys->Dy*(4.0*time+1) ) );
            c_exa[dim1][dim2] = cx*cy/(4.0*time+1.0);
        }
    }
    return;
}

/**
 * @brief
 * Calculate the Norm Error of Numerical Solution at Spicific Time.
 */
void NormErr(structMesh *mesh, physics *phys,
             double time, double **c,
             double *L2, double *Linf){
    
    double **temp = Matrix_create(mesh->ndim1, mesh->ndim2);
    ExactSol(mesh, phys, time, temp);

    double err    = 0;
    double maxerr = 0.0;
    int    dim1, dim2;
    for (dim1=0; dim1<mesh->ndim1; dim1++) {
        for (dim2=0; dim2<mesh->ndim2; dim2++) {
            
            double dc = c[dim1][dim2]-temp[dim1][dim2];
            maxerr = maxf(fabs(dc), maxerr);
            err += dc*dc;
        }
    }
    *L2   = sqrt(err/mesh->ndim1/mesh->ndim2);
    *Linf = maxerr;
    
    Matrix_free(temp);
    return;
}