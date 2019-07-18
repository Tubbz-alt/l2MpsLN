# Release notes

Release notes for the SLAC LCLS-II HPS MPS Link Node

## Releases:

* __l2MpsLN-R2-2-0__: 2019-06-13 J. Vasquez
  * Add support for the LCLS-I operation related registers.

* __l2MpsLN-R2-1-0__: 2019-06-12 J. Vasquez
  * Update l2MpsAsyn to version R2.1.0.
  * Autosave files are build using the info(autosaveFields)
    defined in the databse. Am autosave file, with manually
    defined PVs was added.
  * Add Link Node for building 34.

* __l2MpsLN-R2-0-0__: 2019-02-28 J. Vasquez
  * Update CPSW framework to version R3.6.6, boost to 1.64.0 and
    yaml-cpp to yaml-cpp-0.5.3_boost-1.64.0.
  * Update l2Mps and l2MpsAsyn to version R2.0.0.
  * Update yamlLoader to R1.1.2 and YCPSWASYN to R3.0.5.
  * Use the yamlDownloader to get the YAML files from the FPGA.
  * Use AI record for the ADC values.
  * Set JesdRx.InvertAdcData = 0xF to invert the ADC data and
    revert the effect of the HW inverter present in the ADC card.
  * Add DEV IOC (sioc-b084-mp02).
  * Minor bug fixes and code clean ups.

* __l2MpsLN-R1-0-0__: 2018-07-18 J. Vasquez
  * First stable release for EIC.