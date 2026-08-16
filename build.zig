const std = @import("std");

const c_flags = [_][]const u8{
    "-std=c23",
    "-fblocks",
    "-Wall",
    "-Wextra",
    "-Wpedantic",
    "-Wshadow",
    "-Wconversion",
    "-Wdouble-promotion",
    "-Wformat=2",
    "-Wundef",
};

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});
    const sigma_malloc = b.dependency("sigma_malloc", .{});
    const horny_mode = b.option(bool, "horny-mode", "Enable allocator HORNY_MODE") orelse (optimize == .Debug);
    const no_leak_reward = b.option(bool, "no-leak-reward", "Enable allocator NO_LEAK_REWARD") orelse (optimize == .Debug);
    const use_debug_alloc = b.option(bool, "use-debug-alloc", "Enable allocator debug metadata") orelse (optimize == .Debug);
    const use_malloc_backend = b.option(bool, "malloc-backend", "Use malloc/free as the allocator backing source") orelse false;

    const exe = b.addExecutable(.{
        .name = "sigma_rt",
        .root_module = b.createModule(.{
            .target = target,
            .optimize = optimize,
        }),
    });
    exe.use_llvm = true;
    exe.root_module.link_libc = true;
    exe.root_module.addIncludePath(b.path("include"));
    exe.root_module.addIncludePath(sigma_malloc.path("include"));

    var src_files = std.ArrayList([]const u8).empty;
    defer src_files.deinit(b.allocator);
    findCFiles(b.graph.io, b.allocator, ".", ".", &src_files, false) catch
        @panic("failed to find C files");
    findCFiles(b.graph.io, b.allocator, "src", "src", &src_files, true) catch
        @panic("failed to find C files in src");
    exe.root_module.addCSourceFiles(.{
        .files = src_files.items,
        .flags = &c_flags,
    });

    var sigma_malloc_src_files = std.ArrayList([]const u8).empty;
    defer sigma_malloc_src_files.deinit(b.allocator);
    findCFiles(
        b.graph.io,
        b.allocator,
        sigma_malloc.path("src").getPath(b),
        "src",
        &sigma_malloc_src_files,
        true,
    ) catch @panic("failed to find sigma_malloc C files");
    exe.root_module.addCSourceFiles(.{
        .root = sigma_malloc.path(""),
        .files = sigma_malloc_src_files.items,
        .flags = &c_flags,
    });
    if (horny_mode) exe.root_module.addCMacro("HORNY_MODE", "1");
    if (no_leak_reward) exe.root_module.addCMacro("NO_LEAK_REWARD", "1");
    if (use_debug_alloc) exe.root_module.addCMacro("USE_DEBUG_ALLOC", "1");
    if (use_malloc_backend) exe.root_module.addCMacro("SIGMA_MALLOC_BACKEND", "1");
    b.installArtifact(exe);

    const run = b.addRunArtifact(exe);
    run.step.dependOn(b.getInstallStep());
    if (b.args) |args| run.addArgs(args);
    b.step("run", "Run the sigma_rt example").dependOn(&run.step);

    const map_tests = b.addExecutable(.{
        .name = "map-tests",
        .root_module = b.createModule(.{
            .target = target,
            .optimize = optimize,
        }),
    });
    map_tests.use_llvm = true;
    map_tests.root_module.link_libc = true;
    map_tests.root_module.addIncludePath(b.path("include"));
    map_tests.root_module.addIncludePath(sigma_malloc.path("include"));
    map_tests.root_module.addCSourceFiles(.{
        .files = &.{ "tests/map.c", "src/hash_map.c", "src/libc.c" },
        .flags = &c_flags,
    });
    map_tests.root_module.addCSourceFiles(.{
        .root = sigma_malloc.path(""),
        .files = sigma_malloc_src_files.items,
        .flags = &c_flags,
    });
    map_tests.root_module.addCMacro("SIGMA_MALLOC_BACKEND", "1");

    const env_tests = b.addExecutable(.{
        .name = "env-tests",
        .root_module = b.createModule(.{
            .target = target,
            .optimize = optimize,
        }),
    });
    env_tests.use_llvm = true;
    env_tests.root_module.link_libc = true;
    env_tests.root_module.addIncludePath(b.path("include"));
    env_tests.root_module.addIncludePath(sigma_malloc.path("include"));
    env_tests.root_module.addCSourceFiles(.{
        .files = &.{
            "tests/env.c",
            "src/args.c",
            "src/env.c",
            "src/hash_map.c",
            "src/libc.c",
            "src/rt.c",
        },
        .flags = &c_flags,
    });
    env_tests.root_module.addCSourceFiles(.{
        .root = sigma_malloc.path(""),
        .files = sigma_malloc_src_files.items,
        .flags = &c_flags,
    });
    env_tests.root_module.addCMacro("SIGMA_MALLOC_BACKEND", "1");

    const run_map_tests = b.addRunArtifact(map_tests);
    const run_env_tests = b.addSystemCommand(&.{
        "env",
        "-i",
        "HOME=/tmp",
        "USER=veya",
        "EMPTY=",
        "THING=a=b=c",
    });
    run_env_tests.addArtifactArg(env_tests);
    run_env_tests.expectStdOutEqual("HOME = /tmp\n");

    const test_step = b.step("test", "Run map and environment tests");
    test_step.dependOn(&run_map_tests.step);
    test_step.dependOn(&run_env_tests.step);
}

fn findCFiles(
    io: std.Io,
    allocator: std.mem.Allocator,
    scan_dir_path: []const u8,
    file_dir_path: []const u8,
    files: *std.ArrayList([]const u8),
    recursive: bool,
) !void {
    var dir = std.Io.Dir.cwd().openDir(io, scan_dir_path, .{ .iterate = true }) catch |err| {
        if (err == error.FileNotFound) return;
        return err;
    };
    defer dir.close(io);

    var iter = dir.iterate();
    while (try iter.next(io)) |entry| {
        if (entry.kind == .directory and recursive) {
            if (std.mem.startsWith(u8, entry.name, ".")) continue;
            if (std.mem.eql(u8, entry.name, "zig-out")) continue;

            const scan_sub_path = try std.fmt.allocPrint(allocator, "{s}/{s}", .{ scan_dir_path, entry.name });
            const file_sub_path = try std.fmt.allocPrint(allocator, "{s}/{s}", .{ file_dir_path, entry.name });
            try findCFiles(io, allocator, scan_sub_path, file_sub_path, files, true);
        } else if (std.mem.endsWith(u8, entry.name, ".c")) {
            const path = try std.fmt.allocPrint(allocator, "{s}/{s}", .{ file_dir_path, entry.name });
            try files.append(allocator, path);
        }
    }
}
