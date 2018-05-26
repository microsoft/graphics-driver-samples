# Overview

This project is focused on providing Windows graphics driver samples.

The first sample being developed is a Windows graphics driver for the Raspberry Pi 2.  This is a sample driver intended to demonstrate how to write a graphics driver for the Windows platform.  The driver is a functioning graphics driver on the Raspberry Pi 2 but it is not a fully featured driver.

Work on the driver was started in August 2015.

Anyone wishing to contribute to this project should recognize that this project is managed by a team of professional developers and there is an expectation that all contributing members of community should be be polite, respectful and honest.

All code within this project is covered by the MIT license with Microsoft as the copyright holder.

If you wish to get involved in the project, please email grfxdrvsamples@microsoft.com and include how you wish to contribute.

This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/). For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/) or contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional questions or comments.

# Sample Graphics Driver Status

The sample driver builds and installs on a raspberry pi 2 and supports a variety of operations.  The driver is still in the early stages of development.  The driver is capable of some basic D3D rendering including the ability to run a canonical demo application called Dolphin.

Note, due to limitations of the Raspberry Pi 2 the sample driver running on the Raspberry Pi 2 will never be able to fully support D3D and any usage of the driver must take into account the limitations of the driver and the underlying hardware.

# Roles

##Project Coordinator

There is only one active project coordinator who oversees all aspects of the project.  This role is currently filled by a development engineer at Microsoft.

##Microsoft Developer

Individual Microsoft developers contribute source to the project following the submission process outlined below without further review.

##Solo Developer

Individuals not part of Microsoft contribute source to the project following the submission process outlined below but those submissions will be reviewed by the Project Coordinator before inclusion in the project. It is highly recommended that solo developers work with the project coordinator prior to submission to ensure that their contributions are in aligned with the goals of the project, not already being addressed by other contributors and are likely to be accepted once the work is complete.

# Support

There is currently no active support for how to use the sample driver beyond what is documented in the Wiki.

# Contribution Process By Microsoft Developers

Every Microsoft developer is expected to ensure that the driver compiles (X64&ARM) and successfully passes both RosTest and BasicTests when run on both within a virtual machine (X64) and a Raspberry Pi 2 (ARM).

# Contribution Process By Solo Developers

Solo developers can contribute to the project in various ways including implementing unassigned tasks within an area of the driver, extending the existing testing coverage and adding additional documentation which helps describe the existing drivers functionality.

All submissions by solo developers should be done via a pull request submission that includes a description of the change and for code changes the set of tests that were run.

If a part or all of a submission is rejected, a clear explaination will be provided to the contributor along with guidance on what issues if any can be addressed in order for the submission to be accepted.

The [ISO C++ Core guidelines](https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md) are a good read for anyone hoping to contribute to the project, as we are attempting to follow the bulk of the recommendations especially with respect to resource management. This project uses exceptions and therefore also uses RIAA.

