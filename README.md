# Overview

This project is focused on providing Windows graphics driver samples.

The first sample being developed is a Windows render only sample driver for the
Raspberry Pi 2.

Work on the render only sample driver was started in August 2015 with the
understanding that a fully featured driver may take one to two years to
complete.

Anyone wishing to contribute to this project should recognize that this project
is managed by a team of professional developers and there is an expectation
that all contributing members of community should be be polite, respectful and
honest.

All code within this project is covered by the MIT license with Microsoft as
the copyright holder.

If you wish to get involved in the project, please email <create project DL>
and include how you wish to contribute.

# Roles

##Project Coordinator

There is only one active project coordinator who oversees
all other aspects of the project.  This role is currently filled by the
graphics driver development lead at Microsoft.

##Team Lead

The project has several teams contributing to the project.  In
order to coordinate the activities of these different teams, each team has
assigned a team lead who is responsible for managing and overseeing the
development activities within the team.

Currently, the teams are:
- Microsoft Graphics Driver (MGD) Team
- Microsoft Internet of Things (IOT) Team
- Raspberry Pi Foundation (RPF) Team

##Team Developer

Individual team developers contribute source to the project
following the submission process outlined below without further review.

##Solo Developer

Individuals not part of a team contribute source to the
project following the submission process outlined below but those submissions
will be reviewed by the Project Coordinator before inclusion in the project. It
is highly recommended that solo developers work with the project coordinator
prior to submission to ensure that their contributions are in aligned with the
goals of the project, not already being addressed by other contributors and are
likely to be accepted once the work is complete.

# Support

There is currently no active support for how to use the sample driver.

# Decision-Making Process

Code ownership across the project will be partitioned into areas assigned to
one of the teams.  In general terms, the MGD team will own all generic driver
plumbing dealing with Windows DDI and the RPF team will own all code that is
hardware specfic.  Design and implemenation details and all other related
decisions regarding an area is owned by the corresponding owning team.

Architecture and design decisions that span areas will be discussed among the
teams at a weekly conference call until a general agreement is reached.

# Contribution Process By Team Developers

Every team member is expected to ensure that the driver compiles (X64&ARM)
and successfully passes both RosTest and BasicTests when run on both within a
virtual machine (X64) and a Raspberry Pi 2 (ARM).

# Contribution Process By Solo Developers

Solo developers can contribute to the project in various ways including
implementing unassigned tasks within an area of the driver, extending the 
existing testing coverage and adding additional documentation which helps 
describe the existing drivers functionality.

All submissions by solo developers should be done via a pull request
submission that includes a description of the change and for code changes
the set of tests that were run.

If a part or all of a submission is rejected, a clear explaination will be
provided to the contributor along with guidance on what issues if any can be
addressed in order for the submission to be accepted.
