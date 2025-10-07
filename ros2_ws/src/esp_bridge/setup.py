from setuptools import setup
package_name = 'esp_bridge'
setup(
    name=package_name,
    version='0.0.1',
    packages=[package_name],
    data_files=[
        ('share/ament_index/resource_index/packages', ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='You',
    maintainer_email='you@example.com',
    description='Minimal ROS 2 bridge for CapRig ESP32 example',
    license='MIT',
    entry_points={
        'console_scripts': [
            'ros_side = esp_bridge.ros_side:main',
        ],
    },
)
