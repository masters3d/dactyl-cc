// swift-tools-version: 5.9
import PackageDescription

let allTargets: [Target] = [
        .target(
            name:"glm_keyboard_utilities", 
            path:"src",
            cxxSettings: [
                .unsafeFlags([ "-std=c++17"] ),
                ]
            ),
        .target( 
            name:"KeyboardExtensions", 
            dependencies: ["glm_keyboard_utilities"],
            path:"Sources/KeyboardExtensions", 
            swiftSettings: [.unsafeFlags([
                "-cxx-interoperability-mode=default",
            ])]
        ),
    ]   

let package = Package(
    name: "Swift3dKeyboardKit",
    products: [
        .library(
            name: "Swift3dKeyboardKit", 
            targets: allTargets.filter { $0.type == .regular }.map { $0.name })
    ],
    targets: allTargets
)
