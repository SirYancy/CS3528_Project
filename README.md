# CS3528_Project
Final project for class CS3528
## Team Hope
* Eric Kuha
* Chelsey Riewer
* Robert Susmilch
## Basic Idea
To create a utility application for a (UPS) style package delivery system which operates in one town.
## Requirements
* Sort and order packages into truck
* Determine packages that can be delivered on time
  * Each truck can only operate for 8 hours per day
  * Trucks will have a weight limit
  * Assume it takes 1 minute to drive one block
  * Each stop takes 5 minutes
* Output should include list of packages for each truck and a set of driving directions for each driver.
* Packages have the following information
  * UUID
  * Two client objects: Sender and receiver
  * Priority: Overnight, Two Day, Regular
* Clients have the following information
  * UUID (Maybe?)
  * Name and address information
  * A list record of sent packages
  * A list record of received packages
* The city has the following qualities
  * Assumed to be divided into quadrants
  * X axis is Main Street
  * Y axis is Central Avenue
  * Streets that are y=n are called nth Stree North or nth Street South
  * Streets that are x=n are called nth Avenue East or nth Avenue West
