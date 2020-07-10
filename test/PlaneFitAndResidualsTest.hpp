/*
 * Copyright 2020 © Centre Interdisciplinaire de développement en Cartographie des Océans (CIDCO), Tous droits réservés
 */

/* 
 * File:   PlaneFitAndResidualsTest.hpp
 * Author: Jordan McManus
 */

#ifndef PLANEFITANDRESIDUALSTEST_HPP
#define PLANEFITANDRESIDUALSTEST_HPP

#include "catch.hpp"
#include <cmath>
#include <Eigen/Dense>
#include "../src/math/PlaneFitter.hpp"

TEST_CASE("plane fit and residual test") {

    //create plane params
    //Z = Ax + By + C
    double A = 10;
    double B = 20;
    double C = 30;
    Eigen::Vector3d planeParamsZform(A, B, C);
    
    // general form: ax + by + cz + d = 0 with (a*a + b*b + c*c = 1)
    Eigen::Vector4d planeParamsGeneralForm;
    PlaneFitter::convertPlaneZform2GeneralForm(planeParamsZform, planeParamsGeneralForm);
    
    Eigen::Vector3d testZform;
    PlaneFitter::convertPlaneGeneralForm2Zform(planeParamsGeneralForm, testZform);
    
    double eps = 1e-9;
    REQUIRE(std::abs(testZform(0)-A) < eps);
    REQUIRE(std::abs(testZform(1)-B) < eps);
    REQUIRE(std::abs(testZform(2)-C) < eps);
    
    //create points on plane
    
    double minX = 5.0;
    double maxX = 10.0;
    double minY = 5.0;
    double maxY = 10.0;
    int n = 100;
    
    Eigen::VectorXd x = (0.5*(maxX-minX)*Eigen::VectorXd::Random(n)) + (0.5*(minX+maxX)*Eigen::VectorXd::Ones(n));
    Eigen::VectorXd y = (0.5*(maxY-minY)*Eigen::VectorXd::Random(n)) + (0.5*(minY+maxY)*Eigen::VectorXd::Ones(n));
    Eigen::VectorXd z = A*x + B*y + C*Eigen::VectorXd::Ones(n);
    
    //add noise
    double variance = 0.01;
    Eigen::VectorXd xnoise = variance*Eigen::VectorXd::Random(n);
    Eigen::VectorXd ynoise = variance*Eigen::VectorXd::Random(n);
    Eigen::VectorXd znoise = variance*Eigen::VectorXd::Random(n);
    
    Eigen::VectorXd xn = x + xnoise;
    Eigen::VectorXd yn = y + ynoise;
    Eigen::VectorXd zn = z + znoise;
    
    //fit plane
    Eigen::MatrixXd xyz(n, 3);
    xyz.col(0) = xn;
    xyz.col(1) = yn;
    xyz.col(2) = zn;
    
    Eigen::Vector4d planeParamsGeneralFormEstimation;
    PlaneFitter::fitPlane(xyz, planeParamsGeneralFormEstimation);
    
    //verify that original plane params or obtained
    
    REQUIRE((planeParamsGeneralFormEstimation-planeParamsGeneralForm).norm() < 0.01);
}


#endif /* PLANEFITANDRESIDUALSTEST_HPP */

