export function buildInitialPackageState(packageDefinitions) {
  return Object.fromEntries(
    packageDefinitions.map((packageDefinition) => [packageDefinition.id, packageDefinition.createState()]),
  );
}

export function derivePackageState(packageDefinitions, statusMessage) {
  return Object.fromEntries(
    packageDefinitions.map((packageDefinition) => [packageDefinition.id, packageDefinition.deriveState(statusMessage)]),
  );
}
