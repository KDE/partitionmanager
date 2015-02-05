# collect the needed icon files
set (PARTITIONMANAGER_OXYGEN_ICONS
   16-apps-partitionmanager.png
   22-apps-partitionmanager.png
   32-apps-partitionmanager.png
   48-apps-partitionmanager.png
   64-apps-partitionmanager.png
   128-apps-partitionmanager.png
)
ecm_install_icons(ICONS sc-apps-partitionmanager.svg DESTINATION ${ICON_INSTALL_DIR})
ecm_install_icons(ICONS ${PARTITIONMANAGER_OXYGEN_ICONS} DESTINATION ${ICON_INSTALL_DIR} THEME oxygen)
