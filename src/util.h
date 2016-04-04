/* 
 * File:   util.h
 * Author: ju
 *
 * Created on 04. April 2016, 21:57
 */

#ifndef UTIL_H
#define	UTIL_H

#ifdef	__cplusplus
extern "C" {
#endif

  void getQuadrantCell(int n, int *qx, int *qy);
  void getQuadrantStart(int q, int *qx, int *qy);
  int getAbsoluteX(int q, int qx);
  int getAbsoluteY(int q, int qy);
  int getQuadrantField(int q, int position, int *x, int *y);
  int getQuadrantNr(int x, int y);

#ifdef	__cplusplus
}
#endif

#endif	/* UTIL_H */

